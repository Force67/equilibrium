// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "netlib/NetLib.h"
#include "netlib/Packet.h"
#include "netlib/Peer.h"

#include "flatbuffers/flatbuffers.h"
#include "utility/DetachedQueue.h"

namespace noda {

  using FbsBuffer = flatbuffers::FlatBufferBuilder;

  template <typename T>
  using FbsRef = flatbuffers::Offset<T>;

  struct OutPacket {
	FbsBuffer buffer;
	utility::detached_queue_key<OutPacket> key;
  };

  struct InPacket {
	inline explicit InPacket(netlib::Packet *p) :
	    packet(*p) {}

	netlib::Packet packet;
	utility::detached_queue_key<InPacket> key;
  };
} // namespace noda