// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NetLib.h"
#include "ClientBase.h"

namespace common {

NetClientBase::NetClientBase() {
	// create a new host with max 1 peer and 2 channels
  _host = enet_host_create(nullptr, 1, 2, 0, 0);
}

NetClientBase::~NetClientBase() {
  if(_host)
	enet_host_destroy(_host);
}

bool NetClientBase::SendReliable(uint8_t* ptr, size_t size) {
  auto *packet = enet_packet_create(
	  static_cast<const void *>(ptr),
	  size,
	  ENET_PACKET_FLAG_RELIABLE);

  if(packet) {
	return enet_peer_send(_serverPeer, 1, packet) == 0;
  }

  return false;
}

void NetClientBase::Disconnect() {
  enet_peer_disconnect(_serverPeer, 0xFF);

  while(enet_host_service(_host, &_netEvent, constants::kTimeout) > 0) {
	switch(_netEvent.type) {
	case ENET_EVENT_TYPE_RECEIVE:
	  // while the client dies, we drop every packet
	  enet_packet_destroy(_netEvent.packet);
	  break;
	case ENET_EVENT_TYPE_DISCONNECT:
	  _serverPeer = nullptr;
	  return;
	}
  }

  // kill it by force
  enet_peer_reset(_serverPeer);
  _serverPeer = nullptr;
}
}