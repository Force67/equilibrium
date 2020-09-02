// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "SyncClient.h"

bool SyncClient::connect() { 
  _netClient = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!_netClient) {
    return false;
  }

  // failed to crack address
  if (enet_address_set_host(&_address, kServerIp) < 0) {
    return false;
  }

  _address.port = kServerPort;
  _netServer = enet_host_connect(_netClient, &_address, 2, 0);
  if (!_netServer) {
    return false;
  }

  if (enet_host_service(_netClient, &_netEvent, kTimeout) &&
      _netEvent.type == ENET_EVENT_TYPE_CONNECT) {
  
    // send auth request
    return true;
  }

  enet_peer_reset(_netServer);
  return false;
}

void SyncClient::run() {

}

void SyncClient::disconnect() {
  enet_peer_disconnect(_netServer, 0);

  while (enet_host_service(_netClient, &_netEvent, kTimeout) > 0) {
    switch (_netEvent.type) {
    case ENET_EVENT_TYPE_RECEIVE:
      enet_packet_destroy(_netEvent.packet);
      break;
    case ENET_EVENT_TYPE_DISCONNECT:
      return;
    }
  }

  // by force
  enet_peer_reset(_netServer);
}