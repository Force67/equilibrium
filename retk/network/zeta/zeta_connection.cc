// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <network/zeta/zeta_connection.h>
#include <network/zeta/zeta_packet_writer.h>

namespace network {

ZetaConnection::ZetaConnection(sockpp::inet_address address,
                               ZetaPacketWriter* writer)
    : address_(address), writer_(writer) {}

ZetaConnection::~ZetaConnection() {}

void ZetaConnection::CloseConnection() {
    // quit data.
}

void ZetaConnection::WriteDatagram() {
  writer_->WritePacket(nullptr, 0);
}

void ZetaConnection::ProcessCommand() {
    // handle logic like connection...
    // hello, bye.. logic.
}

}  // namespace network