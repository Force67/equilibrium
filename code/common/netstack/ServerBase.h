// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>
#include <enet/enet.h>

namespace common {

class NetServerBase {
public:
  explicit NetServerBase(uint16_t port);
  ~NetServerBase();

  inline bool Good() const {
	return _host;
  }

  void Listen();

  virtual bool OnConnection(ENetPeer *) = 0;
  virtual bool OnDisconnection(ENetPeer *) = 0;
  virtual void OnPacket(ENetPeer *) = 0;

  void BroadcastReliable(uint8_t *data, size_t len);

private:
  ENetHost *_host;
  ENetEvent _event{};
};
}