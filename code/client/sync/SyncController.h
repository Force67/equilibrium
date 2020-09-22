// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qobject.h>
#include "net/NetBase.h"
#include "utils/Storage.h"
#include "utils/Logger.h"
#include "net/protocol/Message_generated.h"

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

	  // type safe wrapper to guarntee proper specifization
	  template <typename T>
	  inline bool SendFbsPacket(
	      protocol::MsgType tt,
	      const net::FbsOffset<T> ref)
	  {
		return _client->SendFbsPacketReliable(_fbb, tt, ref.Union());
	  }

	  net::FbsBuilder &fbb()
	  {
		return _fbb;
	  }

	signals:
	  void Connected();
	  void Disconnected(uint32_t);

	private:
	  static ssize_t idaapi ProcessorEvent(void *, int, va_list);
	  static ssize_t idaapi IdbEvent(void *, int, va_list);

	  void OnBroadcast(const protocol::Broadcast *);

	  // network events
	  void OnConnectRequest() override;
	  void OnDisconnect(uint32_t) override;
	  void ProcessPacket(uint8_t *, size_t) override;

	  utils::Storage _storage;
	  bool _active = false;
	  QScopedPointer<net::NetClient> _client;

	  net::FbsBuilder _fbb;
	  uint32_t _heartBeatCount = 0;
	};
  } // namespace sync
} // namespace noda