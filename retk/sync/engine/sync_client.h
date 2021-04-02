// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <network/tcp/tcp_client.h>

#include "sync_base.h"

namespace protocol {
struct MessageRoot;
enum MsgType;
}

namespace sync {

class SyncClient final : public network::TCPClient {
 public:
  explicit SyncClient(network::ClientDelegate&);

  void Send(FbsBuffer&, protocol::MsgType, FbsRef<void>);
};
}  // namespace sync