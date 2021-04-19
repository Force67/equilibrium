// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

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

ZetaDatagramQueue::ZetaDatagramQueue() {}
ZetaDatagramQueue::~ZetaDatagramQueue() {
    Clear();
}

void ZetaDatagramQueue::Clear() {

}

}  // namespace network