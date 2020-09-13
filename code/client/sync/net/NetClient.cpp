// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>
#include "NetClient.h"
#include "IdaInc.h"

namespace noda
{
  bool NetClient::_s_socketCreated = false;

  NetClient::NetClient()
  {
  }

  NetClient::~NetClient()
  {
	if(_s_socketCreated) {
	  enet_deinitialize();
	}
  }

  bool NetClient::InitializeNetBase()
  {
	if(!_s_socketCreated) {
	  if(enet_initialize() == 0) {
		_s_socketCreated = true;
		return true;
	  }

	  return false;
	}
	return _s_socketCreated;
  }

  bool NetClient::Connect(const char *address, uint16_t port)
  {
	if(!InitializeNetBase())
	  return false;

	_netClient = enet_host_create(nullptr, 1, 2, 0, 0);
	if(!_netClient) {
	  return false;
	}

	_address.port = port;
	if(!enet_address_set_host(&_address, address) < 0)
	  return false;

	_netServer = enet_host_connect(_netClient, &_address, 2, 0);
	if(!_netServer) {
	  return false;
	}

	// block till we recieve a confirmation of the connection
	if(enet_host_service(_netClient, &_netEvent, kTimeout) &&
	   _netEvent.type == ENET_EVENT_TYPE_CONNECT) {
	  // start the net listener
	  _updateNet = true;
	  QThread::start();
	  return true;
	}

	enet_peer_reset(_netServer);
	return false;
  }

  bool NetClient::SendReliable(uint8_t *ptr, size_t size)
  {
	auto *packet = enet_packet_create(
	    static_cast<const void *>(ptr),
	    size,
	    ENET_PACKET_FLAG_RELIABLE);

	if(packet) {
	  return enet_peer_send(_netServer, 1, packet) == 0;
	}

	return false;
  }

  void NetClient::Disconnect()
  {
	_updateNet = false;
	enet_peer_disconnect(_netServer, 0);

	while(enet_host_service(_netClient, &_netEvent, kTimeout) > 0) {
	  switch(_netEvent.type) {
	  case ENET_EVENT_TYPE_RECEIVE:
		enet_packet_destroy(_netEvent.packet);
		break;
	  case ENET_EVENT_TYPE_DISCONNECT:
		_netServer = nullptr;
		return;
	  }
	}

	// kill it by force
	enet_peer_reset(_netServer);
	_netServer = nullptr;
  }

  void NetClient::run()
  {
	while(_updateNet) {
	  if(enet_host_service(_netClient, &_netEvent, 0) > 0) {
		if(_netEvent.type == ENET_EVENT_TYPE_RECEIVE) {
		  // invalid identifier length
		  if(_netEvent.packet->dataLength < 4) {
			enet_packet_destroy(_netEvent.packet);
			continue;
		  }

		  //TODO: add the message to queue

		  enet_packet_destroy(_netEvent.packet);
		}
	  }

	  QThread::msleep(kNetworkerThreadIdle);
	}
  }
}