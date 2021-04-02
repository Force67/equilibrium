// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "server_impl.h"
#include <sync/protocol/generated/message_root_generated.h>

namespace sync_server {

ServerImpl::ServerImpl(int16_t port)
    : server_(*this),
      timestamp_(std::chrono::high_resolution_clock::now()) {
  server_.TryHost(port == 0 ? sync::kDefaultSyncPort : port);
}

Server::ResultStatus ServerImpl::Initialize(bool withStorage) {
  if (server_.Port() == -1)
    return Server::ResultStatus::kErrorNotInitalized;

  LOG_INFO("Welcome to Sync Server (port: {})", server_.Port());

  if (withStorage) {
    dbService_ = std::make_unique<DataProcessor>(*this);
    const auto res = dbService_->Initialize();
    if (res != DataProcessor::Status::Success) {
      LOG_ERROR("Failed to initialize db service: {}", static_cast<int>(res));
      return Server::ResultStatus::kErrorStorage;
    }
  }

  running_ = true;
  return Server::ResultStatus::kSuccess;
}

void ServerImpl::OnDisconnection(network::PeerId pid, network::QuitReason exitCode) {
  const std::string name = users_.UserById(pid)->Name();

  LOG_INFO("User {} left with reason: {}", name, static_cast<int>(exitCode));
  users_.RemoveUser(pid);

  auto pack = protocol::CreateUserEvent(fbb_, protocol::UserEventType_Quit,
                                users_.UserCount(),
                                        fbb_.CreateString(name));

  server_.Broadcast(protocol::MsgType_UserEvent, fbb_, pack.Union());
}

void ServerImpl::ProcessData(network::PeerId pid, const uint8_t* data, size_t len) {
  const protocol::MessageRoot* root = sync::UnpackMessage(data, len);
  if (!root) {
    LOG_ERROR("ConsumeMessage() -> Failed to unpack message!");
    return;
  }

  LOG_TRACE("ConsumeMessage() -> {}",
            protocol::EnumNameMsgType(root->msg_type()));

  if (root->msg_type() == protocol::MsgType_HandshakeRequest)
    return HandleAuth(pid, root);

  if (dbService_)
    dbService_->UploadMessage(pid, root, len);

  // direct copy of message source.
  server_.BroadcastData(data, len);
}

void ServerImpl::HandleAuth(network::PeerId cid,
                            const protocol::MessageRoot* message) {
  auto* packet = message->msg_as_HandshakeRequest();

  const std::string userName = packet->name()->str();

  if (packet->protocolVersion() < /*network::kClientVersion*/ 1337) {
    server_.DropPeer(cid);
    LOG_WARNING("HandleAuth() -> Dropped client {}:{} for invalid protocolVersion",
                cid, userName);
    return;
  }

  if (packet->token()->str() != _loginToken) {
    server_.DropPeer(cid);
    LOG_WARNING("HandleAuth() -> Dropped client {}:{} for invalid loginToken", cid,
                userName);
    return;
  }

  auto user = users_.AddUser(
      cid, packet->name()->str(), packet->guid()->str());
  LOG_INFO("HandleAuth() -> User {} joined with id {}", user->Name(), cid);

  auto hsAck = protocol::CreateHandshakeAck(
      fbb_, protocol::UserPermissions_NONE, users_.UserCount());
  server_.Send(cid, protocol::MsgType_HandshakeAck, fbb_, hsAck.Union());

  // very big brain
  auto pack = protocol::CreateUserEvent(
      fbb_, protocol::UserEventType_Join, users_.UserCount() + 1,
      fbb_.CreateString(packet->name()->str()));
  server_.Broadcast(protocol::MsgType_UserEvent, fbb_, pack.Union());
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

  server_.Tick();
}
}  // namespace sync_server