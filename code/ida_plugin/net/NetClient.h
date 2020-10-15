// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "protocol/Message_generated.h"
#include "protocol/DisconnectReason_generated.h"

#include "NetBase.h"
#include <qthread.h>

// stupid windows
#ifdef _WIN32
#undef GetMessage
#endif

namespace noda {
  FbsStringRef MakeFbStringRef(FbsBuilder &msg, const QString &other);

  class NetClient : public QThread {
	Q_OBJECT;

  public:
	NetClient(NetDelegate &);
	~NetClient();

	inline bool IsConnected() { return _serverPeer; }

	void Disconnect();
	bool ConnectServer();

	bool SendReliable(uint8_t *, size_t);
	bool SendFbsPacketReliable(FbsBuilder &, protocol::MsgType, const FbsOffset<void>);

  private:
	void SendHandshake();
	void ProcessIncomingPacket(ENetPacket *);

	void run() override;
	bool _updateNet = false;

	ENetHost *_host = nullptr;
	ENetPeer *_serverPeer = nullptr;
	ENetAddress _address{};
	ENetEvent _netEvent{};

	size_t _totalDataRecieved = 0;

  private:
	NetDelegate &_delegate;
  };
} // namespace noda