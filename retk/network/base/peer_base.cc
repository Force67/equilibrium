// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "peer_base.h"
#include <base/logging.h>

namespace network {
using namespace util;

PeerBase::PeerBase(Socket sock, Id identifier, Adress dest_address) : 
    sock(std::move(sock)), id(identifier), address(dest_address) {
}

void PeerBase::Touch() {
  last_seen = util::msec();
}

void PeerBase::Kill() {
  LOG_DCHECK(immortal);

  last_seen = kPeerTimeout + 1s;
}

bool PeerBase::IsDead() {
  return !immortal && (util::msec() - last_seen) > kPeerTimeout;
}
}