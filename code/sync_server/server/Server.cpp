// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <chrono>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include <netlib/NetLib.h>
#include <netlib/Server.h>

#include "Server.h"
#include "Workspace.h"

#include "flatbuffers/flatbuffers.h"
#include "moc_protocol/Message_generated.h"

namespace noda {

  class Server::Impl final : public netlib::ServerBase {
  public:
	explicit Impl(uint16_t port) :
	    _tickTime(std::chrono::high_resolution_clock::now())
	{
	  for(int i = 0; i < 10; i++) {
		if(ServerBase::Host(port))
		  break;

		port++;
	  }
	}

	Server::Status Initialize(bool enabledStorage)
	{
	  if(!ServerBase::Good())
		return Status::NetError;

	  if(enabledStorage) {
		if(!MountProjects(_workspace))
		  return Status::FsError;
	  }

	  _isListening = true;
	  return Status::Success;
	}

	bool OnConnection(ENetPeer *peer) override
	{
	  return true;
	}

	bool OnDisconnection(ENetPeer *) override
	{
	  return true;
	}

	void HandleAuth(const protocol::Message *message)
	{
	  auto *packet = message->msg_as_HandshakeRequest();

	  if(packet->protocolVersion() < netlib::constants::kClientVersion) {
		// kick...
	  }

	  if(packet->token()->str() != _token) {
		// kick...
	  }
	}

	void OnConsume(ENetPeer *source, const uint8_t *data, const size_t len) override
	{
	  flatbuffers::Verifier verifier(data, len);
	  if(!protocol::VerifyMessageBuffer(verifier))
		return;

	  const auto *message = protocol::GetMessage(static_cast<const void *>(data));
	  if(message->msg_type() == protocol::MsgType_HandshakeRequest)
		return HandleAuth(message);

	  if(message->msg_type() == protocol::MsgType_CreateWorkspace) {
		// lookup user permissions.
		auto *packet = message->msg_as_CreateWorkspace();
		CreateWorkspace(_workspace, packet->name()->c_str());
	  }

	  if(message->msg_type() == protocol::MsgType_RemoveWorkspace) {
		// lookup user permissions...
	  }

	  // relay message
	  BroadcastReliable(data, len, source);
	}

	inline void DoUpdate()
	{
	  auto now = std::chrono::high_resolution_clock::now();
	  auto delta = now - _tickTime;
	  _tickTime = now;

	  float deltaMs = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

	  _freeTime += deltaMs;
	  //std::printf("%f\n", _freeTime);

	  //__debugbreak();
	  if(_freeTime > (1000 / 30)) {
		std::printf("Net tick!\n");
		// network tick
		_freeTime = 0;
	  }

	  ServerBase::Listen();
	}

  public:
	bool _isListening = false;
	std::string _token;

	workspace_t _workspace;
	flatbuffers::FlatBufferBuilder _fbb;

	using timestamp_t = std::chrono::high_resolution_clock::time_point;
	timestamp_t _tickTime;
	float _freeTime = 0;
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

  void Server::Tick()
  {
	_impl->DoUpdate();
  }

  bool Server::IsListening() const
  {
	return _impl->_isListening;
  }
} // namespace noda