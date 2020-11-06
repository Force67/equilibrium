// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <netlib/NetLib.h>
#include <netlib/Client.h>

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"
#include "moc_protocol/IdaSync_generated.h"

#include "utility/DetachedQueue.h"

#include <qthread.h>

namespace noda {

  class SyncDelegate {
  public:
	virtual ~SyncDelegate() = default;

	// this is triggered when a connection has been
	// established
	virtual void OnConnected() = 0;

	// We have been dropped
	virtual void OnDisconnect(int reason) = 0;

	// Handle a new message
	virtual void ProcessPacket(const uint8_t *data, size_t size) = 0;
  };

  class Client final : public netlib::Client,
                       public QThread {
  public:
	Client(SyncDelegate &);
	~Client();

	bool ConnectServer();

  private:
	//netlib::Client:
	void OnConnection() override;
	void OnDisconnected(int) override;
	void OnConsume(netlib::Packet *) override;

	//QThread:
	void run() override;

	void HandleAuth(const protocol::Message *message);

	bool _run = false;
	SyncDelegate &_delegate;

  public:
	struct InPacket {
	  inline explicit InPacket(netlib::Packet *p) :
	      packet(*p) {}

	  netlib::Packet packet;
	  utility::detached_queue_key<InPacket> key;
	};

  private:
	utility::detached_mpsc_queue<InPacket> _inQueue;
  };

  inline flatbuffers::Offset<flatbuffers::String>
      MakeFbStringRef(flatbuffers::FlatBufferBuilder &msg, const QString &other)
  {
	const char *str = const_cast<const char *>(other.toUtf8().data());
	size_t sz = static_cast<size_t>(other.size());
	return msg.CreateString(str, sz);
  }

} // namespace noda