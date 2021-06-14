// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/base/peer_base.h>
#include <network/zeta/zeta_protocol.h>

namespace network {

class ZetaDispatcher;

class ZetaPacketReader {
 public:
  explicit ZetaPacketReader(ZetaDispatcher* dispatcher)
      : dispatcher_(dispatcher) {}

  bool ReadAndDispatch(PeerBase& peer);

  static constexpr size_t kMultiFrameCount = 5;

 private:
  // unowned
  ZetaDispatcher* dispatcher_;

  // temp buffer that the socket directly reads into
  uint8_t frame_buffer_[FrameHeader::kMaxSize * kMultiFrameCount]{};
};
}  // namespace network