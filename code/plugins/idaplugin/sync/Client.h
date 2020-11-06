// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <netlib/NetLib.h>
#include <netlib/Client.h>

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"
#include "moc_protocol/IdaSync_generated.h"

#include "Packet.h"
#include "utility/DetachedQueue.h"

#include <qthread.h>

namespace noda {

  class SyncDelegate {
  public:
	virtual ~SyncDelegate() = default;

	// We have been dropped
	virtual void OnDisconnect(int reason) = 0;

	// Handle a new message
	virtual void ProcessPacket(netlib::Packet *) = 0;
  };

  class Client final : public netlib::NetClient,
                       public QThread {
  public:
	Client(SyncDelegate &);
	~Client();

	bool Start();
	void Stop();

	void CreatePacket(
	    protocol::MsgType type,
	    FbsBuffer &buffer,
	    FbsRef<void> packet);

  private:
	//netlib::Client:
	void OnConnection() override;
	void OnDisconnected(int) override;
	void OnConsume(netlib::Packet *) override;

	//QThread:
	void run() override;

	bool _run = false;
	SyncDelegate &_delegate;

  private:
	utility::detached_mpsc_queue<OutPacket> _outQueue;
  };

  inline flatbuffers::Offset<flatbuffers::String>
      MakeFbStringRef(FbsBuffer &msg, const QString &other)
  {
	const char *str = const_cast<const char *>(other.toUtf8().data());
	size_t sz = static_cast<size_t>(other.size());
	return msg.CreateString(str, sz);
  }

} // namespace noda