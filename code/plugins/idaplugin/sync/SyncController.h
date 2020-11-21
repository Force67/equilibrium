// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <QThread>

#include "utils/Logger.h"
#include "utils/NetNode.h"

#include "TaskDispatcher.h"
#include "network/TCPClient.h"

namespace QT {
  class QTimer;
}

namespace noda {
  struct TaskHandler;

  class SyncController final : public QThread,
                               public network::TCPClientConsumer {
	Q_OBJECT;

  public:
	SyncController();
	SyncController(const SyncController &) = delete;
	~SyncController();

	bool Connect();
	void Disconnect();

	bool IsConnected();

	TaskHandler *HanderByNetType(protocol::MsgType);

	void InitializeForIdb();
  signals:
	void Connected();
	void Disconnected(int);
	void Announce(int);

  private:
	void OnAnnouncement(const protocol::Message *);
	void OnProjectJoin(const protocol::Message *);
	void HandleAuth(const protocol::Message *);

	// network events
	void OnDisconnect(int) override;
	void ConsumeMessage(const uint8_t *data, size_t size) override;

	// IDA
	ssize_t HandleEvent(hook_type_t, int, va_list);

	static ssize_t IdbEvent(void *userData, int code, va_list args);
	static ssize_t IdpEvent(void *userData, int code, va_list args);

	enum NodeIndex : nodeidx_t {
		UpdateVersion,
	};

	NetNode _node;

	bool _active = false;

	network::ScopedSocket _sock;
	network::TCPClient _client;

	TaskDispatcher _dispatcher;
	int _localVersion = 0;

	int _userCount = 0;

	using IdaEventType_t = std::pair<hook_type_t, int>;
	std::map<IdaEventType_t, TaskHandler *> _idaEvents;
	std::map<protocol::MsgType, TaskHandler *> _netEvents;
  };
} // namespace noda