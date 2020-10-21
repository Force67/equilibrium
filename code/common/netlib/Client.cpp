// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NetLib.h"
#include "Client.h"

namespace netlib {

  Client::Client()
  {
	// create a new host with max 1 peer and 2 channels
	_host = enet_host_create(nullptr, 1, 2, 0, 0);
  }

  Client::~Client()
  {
	if(_host)
	  enet_host_destroy(_host);
  }

  bool Client::SendReliable(uint8_t *ptr, size_t size)
  {
	auto *packet = enet_packet_create(
	    static_cast<const void *>(ptr),
	    size,
	    ENET_PACKET_FLAG_RELIABLE);

	if(packet) {
	  return enet_peer_send(_serverPeer, 1, packet) == 0;
	}

	return false;
  }

  bool Client::Connect(const char *address, uint16_t port)
  {
	_address.port = port;
	if(enet_address_set_host(&_address, address) < 0)
	  return false;

	_serverPeer = enet_host_connect(_host, &_address, 2, 0);
	return _serverPeer;
  }

  void Client::Disconnect()
  {
	enet_peer_disconnect(_serverPeer, 0xFF);

	while(enet_host_service(_host, &_event, constants::kTimeout) > 0) {
	  switch(_event.type) {
	  case ENET_EVENT_TYPE_RECEIVE:
		// while the client dies, we drop every packet
		enet_packet_destroy(_event.packet);
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

  void Client::Tick()
  {
	if(enet_host_service(_host, &_event, 0) > 0) {
	  switch(_event.type) {
	  case ENET_EVENT_TYPE_CONNECT:
		OnConnection();
		break;
	  case ENET_EVENT_TYPE_DISCONNECT: {
		OnDisconnected(_event.data);
		break;
	  }
	  case ENET_EVENT_TYPE_RECEIVE: {
		_dataRecieved += _event.packet->dataLength;

		Packet packet(_event.packet);
		OnConsume(&packet);
		break;
	  }
	  default:
		break;
	  }
	}
  }
} // namespace netlib