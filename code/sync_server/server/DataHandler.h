// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>

#include "database/StorageModel.h"

#include "utility/DetachedQueue.h"
#include <network/TCPServer.h>

namespace protocol {
  class Message;
}

namespace noda {

  class ServerImpl;

  class DataHandler {
  public:
	enum class Status {
	  Success,
	  HiveError,
	};

	struct Task;

	DataHandler(ServerImpl &);
	~DataHandler();

	Status Initialize();

	void QueueTask(network::connectid_t source, const uint8_t *data, size_t size);

  private:
	void WorkerThread();
	void ProcessTask(Task &);

	// tasks
	void CreateWorkspace(const protocol::Message*);
	void CreateProject(const protocol::Message*);

	//void OpenNodaDb(const NdUser &sender, const protocol::Message *);

  private:
	ServerImpl &_server;

	void SendActionResult(protocol::MsgType, bool failed);

	bool _run = true;
	std::thread _workerThread;

	struct Task {
	  network::connectid_t id;
	  std::unique_ptr<uint8_t[]> data;
	  utility::detached_queue_key<Task> key;
	};
	utility::detached_mpsc_queue<Task> _taskQueue;
	utility::object_pool<Task> _taskPool;

	database::RootDB _mainDb;
  };
} // namespace noda