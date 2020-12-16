// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>

#include "storage/sync_storage_model.h"

#include <network/tcp_server.h>
#include "utility/detached_queue.h"

namespace protocol {
struct MessageRoot;
}

namespace sync_server {

class ServerImpl;

class DataHandler {
 public:
  enum class Status {
    Success,
    HiveError,
  };

  DataHandler(ServerImpl&);
  ~DataHandler();

  Status Initialize();

  void QueueTask(network::connectid_t source, const uint8_t* data, size_t size);

 private:
  struct Task;

  void WorkerThread();
  void ProcessTask(Task&);

  // tasks
  void CreateWorkspace(const protocol::MessageRoot*);
  void CreateProject(const protocol::MessageRoot*);

  // void OpenNodaDb(const NdUser &sender, const protocol::Message *);

 private:
  ServerImpl& _server;

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
}  // namespace sync_server