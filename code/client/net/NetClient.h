// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "protocol/MsgList_generated.h"
#include "protocol/Handshake_generated.h"
#include "protocol/DisconnectReason_generated.h"

#include "NetBase.h"
#include <qthread.h>

namespace noda
{
  namespace net
  {
	FbsStringRef MakeFbStringRef(FbsBuilder &msg, const QString &other);

	class NetClient : public QThread
	{
	  Q_OBJECT;

	public:
	  NetClient(NetDelegate &);
	  ~NetClient();

	  inline bool IsConnected() { return _serverPeer; }

	  void Disconnect();
	  bool ConnectServer();

	  bool SendReliable(uint8_t *, size_t);

	  template <typename T>
	  inline bool SendFBReliable(FbsBuilder &mb, protocol::Data type, const T &data)
	  {
		auto msgRoot = protocol::CreateMessageRoot(mb, type, data.Union());
		mb.Finish(msgRoot);

		return SendReliable(mb.GetBufferPointer(), mb.GetSize());
	  }

	private:
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
  } // namespace net
} // namespace noda