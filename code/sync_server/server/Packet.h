// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "flatbuffers/flatbuffers.h"

#include "utility/DetachedQueue.h"
#include "utility/ObjectPool.h"

#include <netlib/NetLib.h>
#include <netlib/Peer.h>
#include <netlib/Packet.h>

namespace noda {

  using FbsBuffer = flatbuffers::FlatBufferBuilder;

  struct OutPacket {
	inline explicit OutPacket(netlib::connectid_t cid) :
	    id(cid) {}

	netlib::connectid_t id;
	FbsBuffer buffer;
	utility::detached_queue_key<OutPacket> key;
  };

  struct InPacket {
	inline explicit InPacket(netlib::connectid_t cid, netlib::Packet *p) :
	    id(cid), packet(*p) {}

	netlib::connectid_t id;
	netlib::Packet packet;
	utility::detached_queue_key<InPacket> key;
  };
} // namespace noda