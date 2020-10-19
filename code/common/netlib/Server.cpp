// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Server.h"

namespace netlib {

  ServerBase::~ServerBase()
  {
	enet_host_destroy(_host);
  }

  bool ServerBase::Host(uint16_t port)
  {
	ENetAddress address{ 0, port };
	_host = enet_host_create(&address, 10, 0, 0, 0);
	return _host != nullptr;
  }

  void ServerBase::BroadcastReliable(const uint8_t *data, size_t len, PeerBase *ex)
  {
	auto *packet = enet_packet_create(static_cast<const void *>(data), len,
	                                  ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);

	ENetPeer *curPeer = _host->peers;

	for(size_t i = 0; i < _host->peerCount; i++) {
	  if(curPeer != ex->GetPeer())
		enet_peer_send(curPeer, 1, packet);

	  ++curPeer;
	}

	enet_packet_destroy(packet);
  }

  size_t ServerBase::GetPeerCount() const
  {
	return _host->peerCount;
  }

  void ServerBase::Listen()
  {
	if(enet_host_service(_host, &_event, 0) > 0) {
	  switch(_event.type) {
	  case ENET_EVENT_TYPE_CONNECT: {
		PeerBase peer(_event.peer);
		peer.SetId(_event.peer->connectID);
		OnConnection(&peer);
		break;
	  }
	  case ENET_EVENT_TYPE_DISCONNECT: {
		PeerBase peer(_event.peer);
		OnDisconnection(&peer);
		enet_peer_reset(_event.peer);
		break;
	  }
	  case ENET_EVENT_TYPE_RECEIVE: {
		PeerBase peer(_event.peer);
		OnConsume(&peer, _event.packet->data, _event.packet->dataLength);
		enet_packet_destroy(_event.packet);
		break;
	  }
	  }
	}
  }
} // namespace netlib