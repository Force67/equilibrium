// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <qthread.h>
#include <enet/enet.h>

class SyncClient final : public QThread
{
  Q_OBJECT;
public:
  ~SyncClient();

  bool Connect();
  void Disconnect();

private:
  static bool InitializeNetBase();
  static bool _s_socketCreated;

private:
  void ListenNetwork();

  void run() override;
  bool _updateNet = false;

  ENetHost* _netClient;
  ENetPeer* _netServer;
  ENetAddress _address{};
  ENetEvent _netEvent{};
};