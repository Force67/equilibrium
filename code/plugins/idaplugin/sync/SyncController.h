// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <QAtomicInt>
#include <qobject.h>

#include "utils/Logger.h"
#include "utils/AtomicQueue.h"

#include "SyncClient.h"
#include "server/Server.h"

#include "utils/NetNode.h"

namespace QT {
  class QTimer;
}

namespace noda {
  struct SyncHandler;

  class SyncController final : public QObject,
                               public SyncDelegate {
	Q_OBJECT;

  public:
	SyncController();
	SyncController(const SyncController &) = delete;
	~SyncController();

	bool Connect();
	void Disconnect();

	bool CreateLocalHost();
	void DestroyLocalHost();

	bool IsConnected();
	bool IsLocalHosting();

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

	// IDA
	ssize_t HandleEvent(hook_type_t, int, va_list);

	static ssize_t IdbEvent(void *userData, int code, va_list args);
	static ssize_t IdpEvent(void *userData, int code, va_list args);

	NetNode _node;
	bool _active = false;

	netlib::ScopedNetContext _context;
	SyncClient _client;
	std::unique_ptr<noda::Server> _localServer;

	using IdaEventType_t = std::pair<hook_type_t, int>;
	std::map<IdaEventType_t, SyncHandler *> _idaEvents;
	std::map<protocol::MsgType, SyncHandler *> _netEvents;
  };
} // namespace noda