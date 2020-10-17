// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ServerBase.h"

namespace netlib {

  NetServerBase::NetServerBase(uint16_t port)
  {
	ENetAddress address{ 0, port };

	_host = enet_host_create(&address, 10, 0, 0, 0);
  }

  NetServerBase::~NetServerBase()
  {
	enet_host_destroy(_host);
  }

  void NetServerBase::BroadcastReliable(uint8_t *data, size_t len)
  {
	// TODO: think about thread safety???
	for(size_t i = 0; i < _host->peerCount; i++) {
	}
  }

  size_t NetServerBase::GetPeerCount() const {
	return _host->peerCount;
  }

  void NetServerBase::Listen()
  {
	if(enet_host_service(_host, &_event, 0) > 0) {
	  switch(_event.type) {
	  case ENET_EVENT_TYPE_CONNECT:
		break;
	  case ENET_EVENT_TYPE_DISCONNECT: {
		OnDisconnection(_event.peer);
		enet_peer_reset(_event.peer);
		break;
	  }
	  case ENET_EVENT_TYPE_RECEIVE: {
		OnConsume(_event.peer, _event.packet->data, _event.packet->dataLength);
		enet_packet_destroy(_event.packet);
		break;
	  }
	  }
	}
  }
} // namespace netlib