// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "ServerImpl.h"

namespace noda {

  ServerImpl::ServerImpl(int16_t port) :
	  _server(*this),
      _datahandler(*this),
      _tickTime(std::chrono::high_resolution_clock::now())
  {
	  _server.Host(port);
  }

  ServerStatus ServerImpl::Initialize(bool useStorage)
  {
	if(_server.Port() == -1)
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

  void ServerImpl::OnConnection(const network::TCPPeer& peer)
  {
  }

  void ServerImpl::ConsumeMessage(network::TCPPeer& source, const uint8_t* ptr, size_t size)
  {
	flatbuffers::Verifier verifier(ptr, size);
	if(!protocol::VerifyMessageBuffer(verifier))
	  return;

	const auto *message = protocol::GetMessage(static_cast<const void *>(ptr));
	if(message->msg_type() == protocol::MsgType_HandshakeRequest)
	  return HandleAuth(peer, message);

	_datahandler.Queue(peer->Id(), packet);

	BroadcastReliable(packet, peer);
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

  void ServerImpl::OnDisconnection(const network::TCPPeer& peer)
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

	_server.Tick();
  }
} // namespace noda