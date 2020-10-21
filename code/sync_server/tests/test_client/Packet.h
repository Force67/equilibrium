// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "utility/DetachedQueue.h"
#include "utility/ObjectPool.h"

#include <netlib/NetLib.h>
#include <netlib/Peer.h>
#include <netlib/Packet.h>

struct InPacket {
  inline explicit InPacket(netlib::Packet *p) :
      packet(*p) {}

  netlib::Packet packet;
  utility::detached_queue_key<InPacket> key;
};
