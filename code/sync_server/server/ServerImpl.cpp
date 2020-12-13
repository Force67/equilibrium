// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "ServerImpl.h"
#include "utils/Logger.h"

#include "protocol/generated/MessageRoot_generated.h"

namespace noda {

  ServerImpl::ServerImpl(int16_t port) :
      _server(*this),
      _dataHandler(*this),
      _tickTime(std::chrono::high_resolution_clock::now())
  {
	_server.Host(port == 0 ? network::kDefaultServerPort : port);
  }

  ServerStatus ServerImpl::Initialize(bool useStorage)
  {
	if(_server.Port() == -1)
	  return ServerStatus::NetError;

	LOG_INFO("Welcome to Sync Server (port: {})", _server.Port());

	if(useStorage) {
	  // TODO: more result codes
	  const auto res = _dataHandler.Initialize();
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

  void ServerImpl::ConsumeMessage(network::connectid_t cid, const uint8_t *ptr, size_t size)
  {
	flatbuffers::Verifier verifier(ptr, size);
	if(!protocol::VerifyMessageRootBuffer(verifier))
	  return;

	const auto *message = protocol::GetMessageRoot(static_cast<const void *>(ptr));
	if(message->msg_type() == protocol::MsgType_HandshakeRequest)
	  return HandleAuth(cid, message);

	_dataHandler.QueueTask(cid, ptr, size);
	_server.BroadcastPacket(ptr, size, cid);
  }

  void ServerImpl::OnDisconnection(network::connectid_t cid)
  {
	const std::string name = _userRegistry.UserById(cid)->Name();

	LOG_INFO("User {} left", name);
	_userRegistry.RemoveUser(cid);

	for(auto &it : _userRegistry) {
	  // yes this sucks, and buffers should be *refcounted* instead
	  network::FbsBuffer buffer;
	  auto pack = protocol::CreateAnnouncement(buffer,
	                                           protocol::AnnounceType_Disconnect, buffer.CreateString(name));

	  _server.SendPacket(it->Id(), protocol::MsgType_Announcement, buffer, pack.Union());
	}
  }

  void ServerImpl::HandleAuth(network::connectid_t cid, const protocol::MessageRoot *message)
  {
	auto *packet = message->msg_as_HandshakeRequest();

	const std::string userName = packet->name()->str();

	if(packet->protocolVersion() < network::kClientVersion) {
	  _server.DropPeer(cid);
	  LOG_WARNING("HandleAuth: Dropped client {}:{} for invalid protocolVersion", 
		  cid, userName);
	  return;
	}

	if(packet->token()->str() != _loginToken) {
	  _server.DropPeer(cid);
	  LOG_WARNING("HandleAuth: Dropped client {}:{} for invalid loginToken",
	              cid, userName);
	  return;
	}

	for (auto& it : _userRegistry) {
	  network::FbsBuffer buf;
	  auto pack = protocol::CreateUserEvent(
	      buf, protocol::UserEventType_Join,
	      _userRegistry.UserCount() + 1,
	      buf.CreateString(packet->name()->str()));

	  _server.SendPacket(it->Id(), protocol::MsgType_UserEvent, buf, pack.Union());
	}

	userptr_t user = _userRegistry.AddUser(cid, packet->name()->str(), packet->guid()->str());

	LOG_INFO("User {} joined with id {}", user->Name(), cid);

	network::FbsBuffer buffer;
	auto pack = protocol::CreateHandshakeAck(
	    buffer, protocol::UserPermissions_NONE, _userRegistry.UserCount());

	_server.SendPacket(cid, protocol::MsgType_HandshakeAck, buffer, pack.Union());
  }

  void ServerImpl::Update()
  {
	auto now = std::chrono::high_resolution_clock::now();
	auto delta = now - _tickTime;
	_tickTime = now;

	float deltaMs = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

	_freeTime += deltaMs;

	if(_freeTime > (1000 / 30)) {
	  LOG_WARNING("Detected hitch {}", _freeTime);
	  _freeTime = 0;
	}

	_server.Tick();
  }
} // namespace noda