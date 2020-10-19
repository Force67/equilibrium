// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <chrono>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include <netlib/NetLib.h>
#include <netlib/Server.h>

#include "Server.h"
#include "Client.h"
#include "Workspace.h"
#include "Packet.h"

#include "moc_protocol/DisconnectReason_generated.h"
#include "moc_protocol/Message_generated.h"

namespace noda {

  inline utility::object_pool<OutPacket> _outPool;
  inline utility::object_pool<InPacket> _inPool;

  class Server::Impl final : public netlib::Server {
  public:
	explicit Impl(uint16_t port) :
	    _tickTime(std::chrono::high_resolution_clock::now())
	{
	  for(int i = 0; i < 10; i++) {
		if(Server::Host(port))
		  break;

		port++;
	  }
	}

	noda::Server::Status Initialize(bool useStorage)
	{
	  if(!Server::Good())
		return Status::NetError;

	  if(useStorage) {
		if(!MountProjects(_workspace))
		  return Status::FsError;
	  }

	  _isListening = true;
	  return Status::Success;
	}

	void OnDisconnection(netlib::Peer *peer) override
	{
	  auto it = std::find_if(_clientRegistry.begin(), _clientRegistry.end(), [&](clientPtr &cl) {
		return cl->id == peer->Id();
	  });

	  if(it == _clientRegistry.end())
		return;

	  _clientRegistry.erase(it);
	}

	void HandleAuth(netlib::Peer *source, const protocol::Message *message)
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

	  OutPacket *p = _outPool.construct();
	  p->id = id;

	  // TBD:
	  auto pack = protocol::CreateHandshakeAck(p->buffer, protocol::UserPermissions_NONE,
	                                           id, static_cast<int32_t>(_clientRegistry.size()));

	  p->buffer.Finish(protocol::CreateMessage(p->buffer, protocol::MsgType_HandshakeAck, pack.Union()));
	  _outQueue.push(&p->key);
	}

	void OnConsume(netlib::Peer *peer, netlib::Packet *packet) override
	{
	  flatbuffers::Verifier verifier(packet->data(), packet->length());
	  if(!protocol::VerifyMessageBuffer(verifier))
		return;

	  const auto *message = protocol::GetMessage(static_cast<const void *>(packet->data()));
	  if(message->msg_type() == protocol::MsgType_HandshakeRequest)
		return HandleAuth(peer, message);

	  // queue for data thread
	  auto *item = _inPool.construct(peer->Id(), packet);
	  _inQueue.push(&item->key);

	  BroadcastReliable(packet, peer);
	}

	// networking operations only
	inline void ProcessNet()
	{
	  auto now = std::chrono::high_resolution_clock::now();
	  auto delta = now - _tickTime;
	  _tickTime = now;

	  float deltaMs = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

	  _freeTime += deltaMs;

	  if(_freeTime > (1000 / 30)) {
		std::printf("Net tick!\n");
		_freeTime = 0;
	  }

	  // out queue
	  while(auto *packet = _outQueue.pop(&OutPacket::key)) {
		SendReliable(packet->id,
		             packet->buffer.GetBufferPointer(),
		             packet->buffer.GetSize());

		_outPool.destruct(packet);
	  }

	  Server::Listen();
	}

	// expensive operations operating on the database
	inline void ProcessData()
	{
	  while(auto *packet = _inQueue.pop(&InPacket::key)) {
		std::printf("Packet: valid? %d\n",
		            packet->packet.length());
		_inPool.destruct(packet);
	  }
	}

  private:
	void SendPacket(netlib::Peer *peer, protocol::MsgType type, const flatbuffers::Offset<void> packetRef)
	{
	  OutPacket *p = new OutPacket;

	  _outQueue.push(&p->key);
	}

	clientPtr ClientById(netlib::connectid_t cid)
	{
	  auto it = std::find_if(_clientRegistry.begin(), _clientRegistry.end(), [&](clientPtr &cl) {
		return cl->id == cid;
	  });

	  if(it == _clientRegistry.end())
		return {};

	  return *it;
	}

  public:
	bool _isListening = false;
	std::string _token = "";

	workspace_t _workspace;
	flatbuffers::FlatBufferBuilder _fbb;

	using timestamp_t = std::chrono::high_resolution_clock::time_point;
	timestamp_t _tickTime;
	float _freeTime = 0;

	std::vector<clientPtr> _clientRegistry;

	utility::detached_mpsc_queue<OutPacket> _outQueue;
	utility::detached_mpsc_queue<InPacket> _inQueue;
  };

  Server::Server(uint16_t port) :
      _impl{ std::make_unique<Impl>(port) }
  {
  }

  Server::~Server() = default;

  Server::Status Server::Initialize(bool enabledStorage)
  {
	return _impl->Initialize(enabledStorage);
  }

  void Server::ProcessNet()
  {
	_impl->ProcessNet();
  }

  void Server::ProcessData()
  {
	_impl->ProcessData();
  }

  bool Server::IsListening() const
  {
	return _impl->_isListening;
  }
} // namespace noda