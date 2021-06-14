// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/base/peer_base.h>
#include <base/container/quic_circular_deque.h>

namespace network {

class ZetaConnection;

class ZetaDatagramQueue final {
 public:
  using BufferPointer = std::unique_ptr<uint8_t[]>;

  ZetaDatagramQueue(ZetaConnection* connection);
  ~ZetaDatagramQueue();

  bool empty() const { return queue_.empty(); }

  void SendOrQueueDatagram();

  // clear out queue
  void Clear();

 private:
  struct PendingWrite {
    uint32_t length;
    BufferPointer buffer;
    PeerBase::Id target_id;

    PendingWrite();
    PendingWrite(BufferPointer, size_t, PeerBase::Id);
    ~PendingWrite();
    PendingWrite(PendingWrite&& other);
    PendingWrite& operator=(PendingWrite&& other);
  };

  bool cleanup = false;
  quic::QuicCircularDeque<PendingWrite> queue_;

  // unowned:
  ZetaConnection* connection_;
};
}  // namespace network