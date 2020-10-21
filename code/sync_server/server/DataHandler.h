// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>

#include "Packet.h"
#include "moc_protocol/Message_generated.h"

namespace noda {

  class ServerImpl;

  class DataHandler {
  public:
	DataHandler(ServerImpl &);
	~DataHandler();

	void Queue(netlib::connectid_t, netlib::Packet *);

  private:
	void WorkerThread();
	void HandleMessage(const protocol::Message *);

  private:
	ServerImpl &_server;

	bool _run = true;
	std::thread _workerThread;
	utility::detached_mpsc_queue<InPacket> _packetQueue;
  };
} // namespace noda