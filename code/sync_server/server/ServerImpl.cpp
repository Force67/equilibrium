// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "ServerImpl.h"
#include "moc_protocol/DisconnectReason_generated.h"

// stupid windows
#undef GetMessage
#undef GetMessageW

namespace noda {

  inline utility::object_pool<OutPacket> packetPool;

  ServerImpl::ServerImpl(uint16_t port) :
      _datahandler(*this),
      _tickTime(std::chrono::high_resolution_clock::now())
  {
	for(int i = 0; i < 10; i++) {
	  if(Server::Host(port))
		break;

	  port++;
	}
  }

  ServerStatus ServerImpl::Initialize(bool useStorage)
  {
	if(!Server::Good())
	  return ServerStatus::NetError;

	if(useStorage) {
		// TODO: more result codes
	  const auto res = _datahandler.Initialize();
	  switch(res) {
	  case DataHandler::Status::MainDbError:
		return ServerStatus::FsError;
	  default:
		break;
	  }
	}

	_listening = true;
	return ServerStatus::Success;
  }

  void ServerImpl::OnDisconnection(netlib::Peer *peer)
  {
	auto it = std::find_if(_clientRegistry.begin(), _clientRegistry.end(), [&](clientPtr &cl) {
	  return cl->id == peer->Id();
	});

	if(it == _clientRegistry.end())
	  return;

	_clientRegistry.erase(it);
  }

  clientPtr ServerImpl::ClientById(netlib::connectid_t cid)
  {
	auto it = std::find_if(_clientRegistry.begin(), _clientRegistry.end(), [&](clientPtr &cl) {
	  return cl->id == cid;
	});

	if(it == _clientRegistry.end())
	  return {};

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

	const netlib::connectid_t id = source->Id();

	// allocate new user
	auto client = std::make_shared<Client>();
	client->guid = std::move(packet->guid()->str());
	client->name = std::move(packet->name()->str());
	client->id = id;

	_clientRegistry.emplace_back(client);

	FbsBuffer buffer;
	auto pack = protocol::CreateHandshakeAck(
	    buffer, protocol::UserPermissions_NONE,
	    id, static_cast<int32_t>(_clientRegistry.size()));

	CreatePacket(id, protocol::MsgType_HandshakeAck, buffer, pack.Union());
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

	// out queue
	while(auto *packet = _packetQueue.pop(&OutPacket::key)) {
	  SendReliable(packet->id,
	               packet->buffer.GetBufferPointer(),
	               packet->buffer.GetSize());

	  packetPool.destruct(packet);
	}

	Server::Listen();
  }

  void ServerImpl::CreatePacket(netlib::connectid_t cid,
                                protocol::MsgType type,
                                FbsBuffer &buffer,
                                flatbuffers::Offset<void> packet)
  {
	buffer.Finish(protocol::CreateMessage(buffer, type, packet));

	OutPacket *item = packetPool.construct(cid);
	item->buffer = std::move(buffer);

	_packetQueue.push(&item->key);
  }

} // namespace noda