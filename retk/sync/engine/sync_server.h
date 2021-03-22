// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "network/tcp_server.h"
#include <base/detached_queue.h>
#include <flatbuffers/flatbuffers.h>

namespace protocol {
class MessageRoot;
}

namespace sync {
using cid_t = network::connectid_t;

class SyncServerDelegate : public network::TCPServerDelegate {
 public:
  virtual ~SyncServerDelegate() = default;
  virtual void ConsumeMessage(cid_t, const protocol::MessageRoot*) = 0;

 private:
  void ProcessData(cid_t, const uint8_t*, size_t) final override;
};

class SyncServer final : public network::TCPServer {
 public:
  explicit SyncServer(SyncServerDelegate&);
  ~SyncServer();

  void QueuePacket(cid_t cid, flatbuffers::FlatBufferBuilder &fbb);
  void Process();

 public:
  struct Packet;
 private:
  base::detached_mpsc_queue<Packet> queue_;
  SyncServerDelegate& delegate_;
};
}