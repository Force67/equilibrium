// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_client.h"

namespace network {
ZetaClient::ZetaClient() {}

bool ZetaClient::Connect(const PeerBase::Adress& address) {
  connection_ = std::make_unique<ZetaConnection>(address, nullptr);
  packet_reader_ = std::make_unique<ZetaPacketReader>(&dispatcher_);

  return false;
}
}  // namespace network