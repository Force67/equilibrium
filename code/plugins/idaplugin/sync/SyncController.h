// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <QAtomicInt>
#include <qobject.h>

#include "utils/Storage.h"
#include "utils/Logger.h"
#include "utils/AtomicQueue.h"

#include "SyncClient.h"

namespace QT {
  class QTimer;
}

namespace noda {
	struct SyncHandler;

  class SyncController final :
	  public QObject,
	  public SyncDelegate 
  {
	Q_OBJECT;

  public:
	SyncController();
	SyncController(const SyncController &) = delete;
	~SyncController();

	bool ConnectServer();
	void DisconnectServer();

	bool IsConnected();
	// IDA
	ssize_t HandleEvent(hook_type_t, int, va_list);

  signals:
	void Connected();
	void Disconnected(int);
	void Broadcasted(int);

  private:
	void OnAnnouncement(const protocol::Message *);
	void OnProjectJoin(const protocol::Message *);

	// network events
	void OnConnected() override;
	void OnDisconnect(int) override;
	void ProcessPacket(const uint8_t *data, size_t size) override;

	Storage _storage;
	bool _active = false;

	SyncClient _client;

	using IdaEventType_t = std::pair<hook_type_t, int>;
	std::map<IdaEventType_t, SyncHandler *> _idaEvents;
	std::map<protocol::MsgType, SyncHandler *> _netEvents;
  };
} // namespace noda