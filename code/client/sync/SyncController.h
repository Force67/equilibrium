// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "IdaInc.h"
#include <qobject.h>

#include "net/NetBase.h"

namespace noda
{
  namespace net
  {
	class NetClient;
  }

  namespace sync
  {
	class SyncController final : public QObject,
	                             public net::NetDelegate
	{
	  Q_OBJECT;

	public:
	  SyncController();
	  ~SyncController();

	  bool ConnectServer();
	  void DisconnectServer();

	  bool IsConnected();

	signals:
	  void Connected();
	  void Disconnected(uint32_t);

	private:
	  static ssize_t idaapi OnIdaEvent(void *, int, va_list);

	  // network events
	  void OnConnectRequest() override;
	  void OnDisconnect(uint32_t) override;
	  void ProcessPacket(uint8_t *, size_t) override;

	  QScopedPointer<net::NetClient> _client;
	};
  } // namespace sync
} // namespace noda