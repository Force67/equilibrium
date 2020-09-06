// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <qthread.h>
#include <enet/enet.h>

// remote server config
constexpr char kServerIp[] = "127.0.0.1";
constexpr uint16_t kServerPort = 4523;
constexpr uint32_t kTimeout = 3000;

class SyncClient final : public QThread
{
  Q_OBJECT;
public:
  ~SyncClient();

  bool Connect();
  void Disconnect();

  inline bool IsConnected() { 
	  return _netServer;
  }

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
  QString _userName;
};