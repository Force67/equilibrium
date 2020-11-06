// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "ServerImpl.h"
#include "moc_protocol/DisconnectReason_generated.h"

namespace noda {

  inline utility::object_pool<OutPacket> s_packetPool;

  ServerImpl::ServerImpl(uint16_t port) :
      _datahandler(*this),
      _tickTime(std::chrono::high_resolution_clock::now())
  {
	for(int i = 0; i < 10; i++) {
	  if(NetServer::Host(port)) {
		_activePort = port;
		break;
	  }

	  port++;
	}

	NetServer::SetDeleter([](void *data) {
	  OutPacket *packet = static_cast<OutPacket *>(data);
	  s_packetPool.destruct(packet);
	});
  }

  ServerStatus ServerImpl::Initialize(bool useStorage)
  {
	if(!NetServer::Good())
	  return ServerStatus::NetError;

	if(useStorage) {
	  // TODO: more result codes
	  const auto res = _datahandler.Initialize();
	  switch(res) {
	  case DataHandler::Status::HiveError:
		return ServerStatus::FsError;
	  default:
		break;
	  }
	}

	_listening = true;
	return ServerStatus::Success;
  }

  void ServerImpl::OnConnection(netlib::Peer *peer)
  {
  }

  userptr_t ServerImpl::UserById(netlib::connectid_t cid)
  {
	auto it = std::find_if(_userRegistry.begin(), _userRegistry.end(), [&](userptr_t &user) {
	  return user->Id() == cid;
	});

	if(it == _userRegistry.end())
	  return nullptr;

	return *it;
  }

  void ServerImpl::OnConsume(netlib::Peer *peer, netlib::Packet *packet)
  {
	flatbuffers::Verifier verifier(packet->data(), packet->length());
	if(!protocol::VerifyMessageBuffer(verifier))
	  return;

	const auto *message = protocol::GetMessage(static_cast<const void *>(packet->data()));
	if(message->msg_type() == protocol::MsgType_HandshakeRequest)
	  return HandleAuth(peer, message);

	_datahandler.Queue(peer->Id(), packet);

	BroadcastReliable(packet, peer);
  }

  void ServerImpl::HandleAuth(netlib::Peer *source, const protocol::Message *message)
  {
	auto *packet = message->msg_as_HandshakeRequest();

	if(packet->protocolVersion() < netlib::constants::kClientVersion) {
	  source->Kick(protocol::DisconnectReason_BadConnection);
	  return;
	}

	if(packet->token()->str() != _token) {
	  source->Kick(protocol::DisconnectReason_BadPassword);
	  return;
	}

	for(auto &it : _userRegistry) {
	  FbsBuffer buffer;
	  auto pack = protocol::CreateAnnouncement(buffer, protocol::AnnounceType_Joined,
	                                           buffer.CreateString(packet->name()->str()));

	  CreatePacket(it->Id(), protocol::MsgType_Announcement, buffer, pack.Union());
	}

	const netlib::connectid_t id = source->Id();

	userptr_t user = std::make_shared<NdUser>(id,
	                                          packet->name()->str(),
	                                          packet->guid()->str());

	_userRegistry.emplace_back(user);

	FbsBuffer buffer;
	auto pack = protocol::CreateHandshakeAck(
	    buffer, protocol::UserPermissions_NONE,
	    id, static_cast<int32_t>(_userRegistry.size()));

	CreatePacket(id, protocol::MsgType_HandshakeAck, buffer, pack.Union());
  }

  void ServerImpl::OnDisconnection(netlib::Peer *peer)
  {
	auto it = std::find_if(_userRegistry.begin(), _userRegistry.end(), [&](userptr_t &user) {
	  return user->Id() == peer->Id();
	});

	if(it == _userRegistry.end())
	  return;

	std::string name = (*it)->Name();

	_userRegistry.erase(it);

	for(auto &it : _userRegistry) {
	  // yes this sucks, and buffers should be *refcounted* instead
	  FbsBuffer buffer;
	  auto pack = protocol::CreateAnnouncement(buffer,
	                                           protocol::AnnounceType_Disconnect, buffer.CreateString(name));

	  CreatePacket(it->Id(), protocol::MsgType_Announcement, buffer, pack.Union());
	}
  }

  void ServerImpl::Update()
  {
	auto now = std::chrono::high_resolution_clock::now();
	auto delta = now - _tickTime;
	_tickTime = now;

	float deltaMs = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

	_freeTime += deltaMs;

	if(_freeTime > (1000 / 30)) {
	  std::printf("Detected hitch!\n");
	  _freeTime = 0;
	}

	NetServer::Listen();

	// out queue
	while(auto *packet = _packetQueue.pop(&OutPacket::key)) {
	  bool result = SendReliableUnsafe(packet->id,
	                                   packet->buffer.GetBufferPointer(),
	                                   packet->buffer.GetSize(),
	                                   static_cast<void *>(packet));

#if 1
	  if(!result)
		__debugbreak();
#endif
	  // for deletion: visit SetDeleter([](void* data)
	}
  }

  void ServerImpl::CreatePacket(netlib::connectid_t cid,
                                protocol::MsgType type,
                                FbsBuffer &buffer,
                                flatbuffers::Offset<void> packet)
  {
	buffer.Finish(protocol::CreateMessage(buffer, type, packet));

	OutPacket *item = s_packetPool.construct(cid);
	item->buffer = std::move(buffer);

	_packetQueue.push(&item->key);
  }
} // namespace noda