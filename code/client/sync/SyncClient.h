// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <qthread.h>
#include <enet/enet.h>

// remote server config
constexpr char kServerIp[] = "127.0.0.1";
constexpr uint16_t kServerPort = 4523;

// timeout in seconds
constexpr uint32_t kTimeout = 3000;

class SyncClient final : public QThread
{
  Q_OBJECT;
public:
  bool connect();
  void disconnect();

private:
  void run() override;

  ENetHost* _netClient;
  ENetPeer* _netServer;
  ENetAddress _address{};
  ENetEvent _netEvent{};
};