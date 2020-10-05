// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <map>
#include <QAtomicInt>
#include <qobject.h>

#include "net/NetClient.h"

#include "utils/Storage.h"
#include "utils/Logger.h"
#include "utils/AtomicQueue.h"

namespace QT {
  class QTimer;
}

namespace noda {
  class NetClient;
  struct SyncHandler;
  class SyncController;

  struct RequestItem {
	std::unique_ptr<uint8_t[]> data;
	SyncHandler *handler;

	RequestItem()
	{
	  handler = nullptr;
	}
	explicit RequestItem(SyncHandler *, size_t bucketSize);
  };

  struct RequestQueue final : exec_request_t {
	RequestQueue(SyncController &sc) :
	    parent(sc)
	{
	}

	int execute() override;
	void Queue(RequestItem *);

  private:
	AtomicQueue<RequestItem> _queue;
	QAtomicInt _queueLength = 0;
	SyncController &parent;
  };

  class SyncController final : public QObject, public NetDelegate {
	Q_OBJECT;

  public:
	SyncController();
	SyncController(const SyncController &) = delete;
	~SyncController();

	bool ConnectServer();
	void DisconnectServer();

	bool IsConnected();

	// type safe wrapper to guarantee proper specification
	template <typename T>
	inline bool SendFbsPacket(protocol::MsgType tt, const FbsOffset<T> ref)
	{
	  return _client->SendFbsPacketReliable(_fbb, tt, ref.Union());
	}

	// Get the packet builder
	auto &fbb()
	{
	  return _fbb;
	}

	// Get Net Stats
	auto &stats() const
	{
	  return netStats;
	}

	// IDA
	ssize_t HandleEvent(hook_type_t, int, va_list);

  signals:
	void Connected();
	void Disconnected(uint32_t);
	void Broadcasted(int);
	void StatsUpdated(const NetStats &);

  private:
	void OnAnnouncement(const protocol::Message *);
	void OnProjectJoin(const protocol::Message *);

	// network events
	void OnConnected() override;
	void OnDisconnect(uint32_t) override;
	void ProcessPacket(const uint8_t *data, size_t size) override;

	Storage _storage;
	bool _active = false;
	QScopedPointer<NetClient> _client;

	FbsBuilder _fbb;
	uint32_t _heartBeatCount = 0;

	using IdaEventType_t = std::pair<hook_type_t, int>;
	std::map<IdaEventType_t, SyncHandler *> _idaEvents;
	std::map<protocol::MsgType, SyncHandler *> _netEvents;

	QScopedPointer<QTimer> _statsTimer;
	RequestQueue _requestQueue;
  };
} // namespace noda