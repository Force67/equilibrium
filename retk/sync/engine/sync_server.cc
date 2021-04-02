// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sync_server.h"
#include <base/object_pool.h>

#include "protocol/generated/message_root_generated.h"

namespace sync {

SyncServer::SyncServer(network::ServerDelegate& d) : network::TCPServer(d) {}

SyncServer::~SyncServer() {}

void SyncServer::QueueFbsCommand(network::PeerId pid, FbsBuffer& buffer) {
  QueueCommand(network::CommandId::kData, pid, buffer.GetBufferPointer(),
               buffer.GetSize());
}

void SyncServer::Broadcast(protocol::MsgType type,
                           FbsBuffer& buf,
                           FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buf, type, ref);
  buf.Finish(packet);

  QueueFbsCommand(network::kAllConnectId, buf);
}

void SyncServer::BroadcastData(const uint8_t* data, size_t len) {
  QueueCommand(network::CommandId::kData, network::kAllConnectId, data, len);
}

void SyncServer::Send(network::PeerId pid,
                      protocol::MsgType type,
                      FbsBuffer& buffer,
                      FbsRef<void> ref) {
  const auto packet = protocol::CreateMessageRoot(buffer, type, ref);
  buffer.Finish(packet);

  // TODO: think about moving to sync_message_builder
  QueueFbsCommand(pid, buffer);
}
}  // namespace sync