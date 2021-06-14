// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_protocol.h"
#include "zeta_dispatcher.h"
#include "zeta_packet_reader.h"

#include <base/logging.h>

namespace network {

bool ZetaPacketReader::ReadAndDispatch(PeerBase& peer) {
  PeerBase::Socket& socket = peer.sock;

  // reset all buffers for reuse
  std::memset(&frame_buffer_, 0, sizeof(frame_buffer_));

  // read multiple packets in one go
  size_t packets_read = 0;
  while (socket.read_n(&frame_buffer_[packets_read * FrameHeader::kMaxSize],
                       sizeof(FrameHeader::kMaxSize)) > 0 &&
         packets_read < kMultiFrameCount) {
    packets_read++;
  }

  for (size_t i = 0; i < packets_read; i++) {
    const uint8_t* buffer = &frame_buffer_[i * FrameHeader::kMaxSize];

    const FrameHeader* header = reinterpret_cast<const FrameHeader*>(buffer);

    if (header->magic != FrameHeader::kMagic) {
      LOG_ERROR("Not a zeta packet!");
      continue;
    }

    if (header->type >= FrameType::kNumMessages ||
        header->type <= FrameType::kInvalid) {
      LOG_WARNING("Invalid frame type: {}", static_cast<int>(header->type));
      continue;
    }

    dispatcher_->ProcessPacket(
        header->type, peer.id, buffer + sizeof(FrameHeader),
        static_cast<size_t>(header->size - sizeof(FrameHeader)));
  }

  return true;
}
}  // namespace network