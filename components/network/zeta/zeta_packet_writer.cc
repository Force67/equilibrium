// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <memory>
#include "zeta_protocol.h"
#include "zeta_packet_writer.h"

namespace network {

bool ZetaPacketWriter::WritePacket(PeerBase &peer, const uint8_t* buf, size_t length) {
	const size_t full_size = sizeof(FrameHeader) + length;
	auto out_buf_ = std::make_unique<uint8_t[]>(full_size);
	// assemble header in place
	FrameHeader* header = new (out_buf_.get()) FrameHeader();
	std::memcpy(&out_buf_[sizeof(FrameHeader)], static_cast<const void*>(buf), length);

	return peer.sock.write_n(out_buf_.get(), full_size) > 0;
}
}