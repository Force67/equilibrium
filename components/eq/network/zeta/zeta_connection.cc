// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <network/zeta/zeta_connection.h>
#include <network/zeta/zeta_packet_writer.h>
#include <network/zeta/zeta_packet_reader.h>
#include <base/logging.h>

namespace network {

ZetaConnection::ZetaConnection(sockpp::inet_address address,
                               ZetaPacketWriter* writer)
    : PeerBase(1337, address), address_(address), writer_(writer) {}

ZetaConnection::~ZetaConnection() {}

void ZetaConnection::CloseConnection() {
  // quit data.
  NotifyQuit quitCommand{NotifyQuit::Reason::kQuit};
  DoSubmitDatagram(quitCommand);
}

void ZetaConnection::WriteDatagram(const uint8_t* ptr, size_t length) {
  writer_->WritePacket(*this, ptr, length);
}

void ZetaConnection::ProcessCommand(FrameType type,
                                    const uint8_t* ptr,
                                    size_t length) {
  switch (type) {
    case FrameType::kHello: {
      // received authentication challenge
      break;
    }
    case FrameType::kBye: {
      // received quit
      break;
    }
    default:
    case FrameType::kInvalid: {
      LOG_ERROR("ZetaConnection::ProcessCommand() -> Invalid frame type {}",
                static_cast<int>(type));
      return;
    }
  }
}

}  // namespace network