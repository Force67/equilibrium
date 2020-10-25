// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>

#include "Packet.h"
#include "moc_protocol/Message_generated.h"

#include "database/SqliteDB.h"

namespace noda {

  class ServerImpl;

  class DataHandler {
  public:
	enum class Status {
	  Success,
	  MainDbError,
	};

	DataHandler(ServerImpl &);
	~DataHandler();

	Status Initialize();

	void Queue(netlib::connectid_t, netlib::Packet *);

  private:
	void WorkerThread();
	void HandleMessage(const protocol::Message *);

	void CreateWks(const protocol::Message *);

  private:
	ServerImpl &_server;

	bool _run = true;
	std::thread _workerThread;
	utility::detached_mpsc_queue<InPacket> _packetQueue;

	database::SqliteDB _db;
  };
} // namespace noda