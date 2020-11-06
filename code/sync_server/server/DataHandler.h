// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>

#include "Packet.h"
#include "moc_protocol/Message_generated.h"

#include "Storage.h"

namespace noda {

  class ServerImpl;

  class DataHandler {
  public:
	enum class Status {
	  Success,
	  HiveError,
	};

	DataHandler(ServerImpl &);
	~DataHandler();

	Status Initialize();

	void Queue(netlib::connectid_t, netlib::Packet *);

  private:
	void WorkerThread();

	void CreateBucket(const protocol::Message *);
	void DeleteBucket(const protocol::Message *);

	void OpenNodaDb(const NdUser &sender, const protocol::Message *);

  private:
	ServerImpl &_server;

	void SendActionResult(protocol::MsgType, bool failed);

	bool _run = true;
	std::thread _workerThread;
	utility::detached_mpsc_queue<InPacket> _packetQueue;

	Storage _storage;
	std::vector<NodaDb> dbref_;
  };
} // namespace noda