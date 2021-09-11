// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "zeta_server.h"

namespace network {

ZetaServer::ZetaServer() {
	packet_reader_ = std::make_unique<ZetaPacketReader>(&dispatcher_);
}

ZetaServer::~ZetaServer() {

}

bool ZetaServer::TickFrame() {
	// while socket
	// from peer
	packet_reader_->ReadAndDispatch();
}
}