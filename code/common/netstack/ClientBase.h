// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace common {

class NetClientBase {
public:
  NetClientBase();
  ~NetClientBase();

  inline bool Good() const {
	return _host;
  }
  
  bool SendReliable(uint8_t *ptr, size_t size);

  void Disconnect();
private:
  ENetHost *_host = nullptr;
  ENetPeer *_serverPeer = nullptr;
  ENetAddress _address{};
  ENetEvent _netEvent{};
};
}