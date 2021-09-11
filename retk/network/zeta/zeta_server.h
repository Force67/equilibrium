// Copyright (C) 2021 Force67 <github.com/Force67>.
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
		ZetaDispatcher dispatcher_;
		std::unique_ptr<ZetaPacketReader> packet_reader_;
	};
}