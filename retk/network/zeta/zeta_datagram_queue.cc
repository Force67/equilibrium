// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_connection.h"
#include "zeta_datagram_queue.h"

namespace network {

ZetaDatagramQueue::PendingWrite::PendingWrite() = default;

ZetaDatagramQueue::PendingWrite::PendingWrite(
    ZetaDatagramQueue::BufferPointer producer,
    size_t size,
    PeerBase::Id peer_id)
    : length(static_cast<uint32_t>(size)),
      buffer(std::move(producer)),
      target_id(peer_id) {}

ZetaDatagramQueue::PendingWrite::~PendingWrite() = default;

ZetaDatagramQueue::PendingWrite::PendingWrite(PendingWrite&& other) = default;

ZetaDatagramQueue::PendingWrite& ZetaDatagramQueue::PendingWrite::operator=(
    PendingWrite&& other) = default;

ZetaDatagramQueue::ZetaDatagramQueue(ZetaConnection* connection)
    : connection_(connection) {}

ZetaDatagramQueue::~ZetaDatagramQueue() {
  Clear();
}

void ZetaDatagramQueue::Clear() {}

void ZetaDatagramQueue::SendOrQueueDatagram() {
  if (queue_.empty()) {
    connection_->WriteDatagram(nullptr, 0);
    return;
  }

  queue_.emplace_back(PendingWrite());
}

// https://github.com/chromium/chromium/blob/master/base/containers/circular_deque.h
// https://quiche.googlesource.com/quiche/+/refs/heads/master/quic/core/quic_datagram_queue.cc
// https://quiche.googlesource.com/quiche/+/refs/heads/master/quic/core/quic_connection.h

}  // namespace network