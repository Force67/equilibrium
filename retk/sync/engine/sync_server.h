// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <network/tcp/tcp_server.h>

#include "sync_base.h"

namespace protocol {
struct MessageRoot;
enum MsgType;
}  // namespace protocol

namespace sync {

class SyncServer final : public network::TCPServer {
 public:
  explicit SyncServer(network::ServerDelegate&);
  ~SyncServer();

  void Broadcast(protocol::MsgType, FbsBuffer&, FbsRef<void> ref);
  void BroadcastData(const uint8_t*, size_t);

  void Send(network::PeerId, protocol::MsgType, FbsBuffer&, FbsRef<void>);

 private:
  void QueueFbsCommand(network::PeerId, FbsBuffer&);
};
}  // namespace sync