// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/zeta/zeta_dispatcher.h>
#include <network/zeta/zeta_packet_reader.h>

namespace network {

class ZetaServer final {
 public:
  ZetaServer();
  ~ZetaServer();

  bool TickFrame();

 private:
	 // dispatcher owns peer collection.
  ZetaDispatcher dispatcher_;
  std::unique_ptr<ZetaPacketReader> packet_reader_;
};
}  // namespace networ