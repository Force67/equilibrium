// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "network/tcp_client.h"
#include <base/detached_queue.h>

#include "sync_base.h"

namespace protocol {
struct MessageRoot;
enum MsgType;
}

namespace sync {

class SyncClientDelegate : public network::TCPClientDelegate {
 public:
  virtual ~SyncClientDelegate() = default;
  virtual void ConsumeMessage(const protocol::MessageRoot*, size_t len) = 0;

 private:
  void ProcessData(const uint8_t* ptr, size_t len) final override;
};

class SyncClient final : public network::TCPClient {
 public:
  explicit SyncClient(SyncClientDelegate&);

  void Send(protocol::MsgType type, FbsBuffer& buf, FbsRef<void> ref);
  void Process();

  struct Packet;
 private:
  base::detached_mpsc_queue<Packet> queue_;
  uint32_t userCount_ = 1;
  SyncClientDelegate& delegate_;
};
}  // namespace sync