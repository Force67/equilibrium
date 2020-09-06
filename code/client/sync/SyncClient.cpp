// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include <qsettings.h>
#include "SyncClient.h"

bool SyncClient::_s_socketCreated = false;

SyncClient::~SyncClient() { 
    if (_s_socketCreated) {
      enet_deinitialize();
    }
}

bool SyncClient::InitializeNetBase() { 
  if (!_s_socketCreated) {
    if (enet_initialize() == 0) {
      _s_socketCreated = true;
      return true;
    }

    return false;
  }
  return _s_socketCreated;
}

bool SyncClient::Connect() { 
  if (!InitializeNetBase())
    return false;

  _netClient = enet_host_create(nullptr, 1, 2, 0, 0);
  if (!_netClient) {
    return false;
  }

  QSettings settings;
  auto &ip = settings.value("NODASyncIp", kServerIp).toString();
  auto port = settings.value("NODASyncPort", kServerPort).toUInt();

  if (!enet_address_set_host(&_address, ip.toLocal8Bit().data()))
    return false;

  _address.port = static_cast<uint16_t>(port);

  _netServer = enet_host_connect(_netClient, &_address, 2, 0);
  if (!_netServer) {
    return false;
  }

  if (enet_host_service(_netClient, &_netEvent, kTimeout) &&
      _netEvent.type == ENET_EVENT_TYPE_CONNECT) {
    QThread::start();
    return true;
  }

  enet_peer_reset(_netServer);
  return false;
}

void SyncClient::Disconnect() {
  _updateNet = false;
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

  // kill it by force
  enet_peer_reset(_netServer);
  _netServer = nullptr;
}

void SyncClient::ListenNetwork() {
  while (enet_host_service(_netClient, &_netEvent, 0) > 0) {
    switch (_netEvent.type) {
    case ENET_EVENT_TYPE_CONNECT: {

      printf("[CLI] A new client connected from %x:%u.\n",
             _netEvent.peer->address.host, _netEvent.peer->address.port);
      _netEvent.peer->data = nullptr;
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT: {
      printf("[CLI] %s disconnected.\n", _netEvent.peer->data);
      _netEvent.peer->data = NULL;
      break;
    }
    case ENET_EVENT_TYPE_RECEIVE: {
      // invalid identifier length
      if (_netEvent.packet->dataLength < 4) {
        enet_packet_destroy(_netEvent.packet);
        break;
      }

      printf("[CLI] A packet of length %u containing %s was received from %s "
             "on channel %u.\n",
             _netEvent.packet->dataLength, _netEvent.packet->data,
             _netEvent.peer->data, _netEvent.channelID);

      enet_packet_destroy(_netEvent.packet);
      break;
    }
    }
  }
}

void SyncClient::run() { 
  while (_updateNet) {
    ListenNetwork();
    QThread::msleep(1);
  }
}