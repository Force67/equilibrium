// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

// The entity that handles messages for a Zeta client or server
// On the client side, the Connection handles raw reads & processing.
#pragma once

#include <sockpp/inet_address.h>

namespace network {

class ZetaPacketWriter;

class ZetaConnection final {
 public:
  explicit ZetaConnection(sockpp::inet_address, ZetaPacketWriter* writer);
  ~ZetaConnection();

  void WriteDatagram();

  void CloseConnection();

  void ProcessCommand();

  bool connected() const { return connected_; }
 private:
  bool connected_ = false;
  sockpp::inet_address address_;

  // unowned
  ZetaPacketWriter* writer_;
};
}