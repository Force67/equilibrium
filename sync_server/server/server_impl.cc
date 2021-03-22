// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "server_impl.h"
#include "utils/server_logger.h"

#include <sync/protocol/generated/message_root_generated.h>

namespace sync_server {

ServerImpl::ServerImpl(int16_t port)
    : server_(*this),
      _dataHandler(*this),
      timestamp_(std::chrono::high_resolution_clock::now()) {
  server_.Host(port == 0 ? network::kDefaultServerPort : port);
}

Server::ResultStatus ServerImpl::Initialize(bool useStorage) {
  if (server_.Port() == -1)
    return Server::ResultStatus::kErrorNotInitalized;

  LOG_INFO("Welcome to Sync Server (port: {})", _server.Port());

  if (useStorage) {
    // TODO: more result codes
    const auto res = _dataHandler.Initialize();
    switch (res) {
      case DataHandler::Status::HiveError:
        return Server::ResultStatus::kErrorStorage;
      default:
        break;
    }
  }

  running_ = true;
  return Server::ResultStatus::kSuccess;
}

void ServerImpl::ConsumeMessage(sync::cid_t sorse2, const protocol::MessageRoot* root) {
  LOG_TRACE("ConsumeMessage() -> {}",
            protocol::EnumNameMsgType(root->msg_type()));

  if (root->msg_type() == protocol::MsgType_HandshakeRequest)
    return HandleAuth(sorse2, root);

  // this bit sucks 
  _dataHandler.QueueTask(cid, ptr, size);
  _server.BroadcastPacket(ptr, size, cid);
}

void ServerImpl::OnDisconnection(network::connectid_t cid) {
  const std::string name = _userRegistry.UserById(cid)->Name();

  LOG_INFO("User {} left", name);
  _userRegistry.RemoveUser(cid);

  for (auto& it : _userRegistry) {
    // yes this sucks, and buffers should be *refcounted* instead
    auto pack = protocol::CreateAnnouncement(
        fbb_, protocol::AnnounceType_Disconnect, fbb_.CreateString(name));

    _server.SendPacket(it->Id(), protocol::MsgType_Announcement, buffer,
                       pack.Union());
  }
}

void ServerImpl::HandleAuth(network::connectid_t cid,
                            const protocol::MessageRoot* message) {
  auto* packet = message->msg_as_HandshakeRequest();

  const std::string userName = packet->name()->str();

  if (packet->protocolVersion() < network::kClientVersion) {
    server_.DropPeer(cid);
    LOG_WARNING("HandleAuth: Dropped client {}:{} for invalid protocolVersion",
                cid, userName);
    return;
  }

  if (packet->token()->str() != _loginToken) {
    server_.DropPeer(cid);
    LOG_WARNING("HandleAuth: Dropped client {}:{} for invalid loginToken", cid,
                userName);
    return;
  }

  flatbuffers::FlatBufferBuilder buf;

  for (auto& it : _userRegistry) {
    auto pack = protocol::CreateUserEvent(
        buf, protocol::UserEventType_Join, _userRegistry.UserCount() + 1,
        buf.CreateString(packet->name()->str()));

    _server.SendPacket(it->Id(), protocol::MsgType_UserEvent, buf,
                       pack.Union());
  }

  userptr_t user =
      _userRegistry.AddUser(cid, packet->name()->str(), packet->guid()->str());

  LOG_INFO("User {} joined with id {}", user->Name(), cid);

  auto pack = protocol::CreateHandshakeAck(
      buf, protocol::UserPermissions_NONE, _userRegistry.UserCount());

  _server.SendPacket(cid, protocol::MsgType_HandshakeAck, buf, pack.Union());
}

void ServerImpl::Update() {
  auto now = std::chrono::high_resolution_clock::now();
  auto delta = now - timestamp_;
  timestamp_ = now;

  float deltaMs =
      std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();

  yieldTime_ += deltaMs;

  if (yieldTime_ > (1000 / 30)) {
    LOG_WARNING("Detected hitch {}", yieldTime_);
    yieldTime_ = 0;
  }

  server_.Process();
}
}  // namespace sync_server