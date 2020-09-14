// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qthread.h>
#include <enet/enet.h>

namespace noda
{
  // default values
  constexpr char kServerIp[] = "127.0.0.1";
  constexpr uint16_t kServerPort = 4523;

  // constants
  constexpr uint32_t kTimeout = 3000;
  constexpr uint32_t kNetworkerThreadIdle = 1;
  constexpr uint16_t kClientVersion = 1;

  class NetClient : public QThread
  {
	Q_OBJECT;

  public:
	NetClient();
	~NetClient();

	inline bool IsConnected() { return _netServer; }

	bool SendReliable(uint8_t *, size_t);

  protected:
	bool Connect(const char *, uint16_t);
	void Disconnect();

  private:
	static bool InitializeNetBase();
	static bool _s_socketCreated;

  private:
	void run() override;
	bool _updateNet = false;

	ENetHost *_netClient = nullptr;
	ENetPeer *_netServer = nullptr;
	ENetAddress _address{};
	ENetEvent _netEvent{};
  };
}