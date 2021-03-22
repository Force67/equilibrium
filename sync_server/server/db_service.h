// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <thread>
#include <base/detached_queue.h>
#include <sync/engine/sync_server.h>

namespace protocol {
struct MessageRoot;
}

namespace sync::storage {
class MainDb;
class ProjectDb;
}

namespace sync_server {

class ServerImpl;

class DbService {
 public:
  enum class Status {
    Success,
    HiveError,
  };

  DbService(ServerImpl&);
  ~DbService();

  Status Initialize();

  void UploadMessage(sync::cid_t src, const protocol::MessageRoot*, size_t len);

  struct Tasklet;
 private:
  void WorkerThread();
  void ProcessTask(Tasklet&);

  // tasks
  void CreateWorkspace(const protocol::MessageRoot*);
  void CreateProject(const protocol::MessageRoot*);

  // void OpenNodaDb(const NdUser &sender, const protocol::Message *);

 private:
  ServerImpl& _server;

  bool running_ = true;
  std::thread worker_;
  sync::FbsBuffer fbb_;
  std::unique_ptr<sync::storage::MainDb> maindb_;
  //std::unique_ptr<sync::storage::ProjectDb> project_;
  base::detached_mpsc_queue<Tasklet> queue_;
};
}  // namespace sync_server