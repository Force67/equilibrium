// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>
#include <network/tcp_stack/tcp_server.h>

#include "sync_base.h"

namespace protocol {
struct MessageRoot;
enum MsgType;
}

namespace sync {
using cid_t = network::connectid_t;
using FbsBuffer = flatbuffers::FlatBufferBuilder;

class SyncServerDelegate : public network::TCPServerDelegate {
 public:
  virtual ~SyncServerDelegate() = default;
  virtual void ConsumeMessage(cid_t, const protocol::MessageRoot*, size_t) = 0;

 private:
  void ProcessData(cid_t, const uint8_t*, size_t) final override;
};

class SyncServer final : public network::TCPServer {
 public:
  explicit SyncServer(SyncServerDelegate&);
  ~SyncServer();

  void Broadcast(protocol::MsgType, FbsBuffer&, FbsRef<void> ref);
  void Broadcast(const protocol::MessageRoot*, size_t);
  void Send(cid_t cid, protocol::MsgType, FbsBuffer&, FbsRef<void> ref);

  void Process();

 public:
  struct Packet;
 private:
  base::detached_mpsc_queue<Packet> queue_;
  SyncServerDelegate& delegate_;
};
}