// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <QAtomicInt>
#include <qobject.h>

#include "utils/Logger.h"
#include "utils/AtomicQueue.h"
#include "utils/NetNode.h"

#include "Client.h"
#include "Packet.h"
#include "LocalServer.h"

namespace QT {
  class QTimer;
}

namespace noda {
  struct SyncHandler;

  class SyncController final : public QThread,
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
	void Announce(int);

  private:
	void OnAnnouncement(const protocol::Message *);
	void OnProjectJoin(const protocol::Message *);
	void HandleAuth(const protocol::Message*);

	// network events
	void OnDisconnect(int) override;
	void ProcessPacket(netlib::Packet *) override;

	// IDA
	ssize_t HandleEvent(hook_type_t, int, va_list);

	static ssize_t IdbEvent(void *userData, int code, va_list args);
	static ssize_t IdpEvent(void *userData, int code, va_list args);

	NetNode _node;
	bool _active = false;

	netlib::ScopedNetContext _context;

	Client _client;
	int _userCount = 0;
	//LocalServer _server;

	using IdaEventType_t = std::pair<hook_type_t, int>;
	std::map<IdaEventType_t, SyncHandler *> _idaEvents;
	std::map<protocol::MsgType, SyncHandler *> _netEvents;

	// dispatch events on ida thread
	struct Dispatcher : exec_request_t {
	  Dispatcher(SyncController &sc) :
	      sc(sc) {}

	  SyncController &sc;
	  int idaapi execute() override;
	} _dispatcher;

	std::atomic_int _eventSize = 0;
	utility::detached_mpsc_queue<InPacket> _queue;
  };
} // namespace noda