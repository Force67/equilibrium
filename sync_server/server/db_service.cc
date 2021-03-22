// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "db_service.h"
#include "server_impl.h"

#include <base/thread.h>
#include "utils/server_logger.h"

#include <sync/storage/main_db.h>
#include <sync/protocol/generated/message_root_generated.h>

using namespace std::chrono_literals;

namespace fs = std::filesystem;

namespace sync_server {

static const fs::path& GetStoragePath() noexcept {
  static fs::path s_path{};
  if (s_path.empty()) {
    s_path = fs::current_path() / "storage";
    if (!fs::exists(s_path))
      fs::create_directory(s_path);
  }

  return s_path;
}

struct DbService::Tasklet {
  sync::cid_t source;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Tasklet> key;
};

static base::object_pool<DbService::Tasklet> s_Pool;

DbService::DbService(ServerImpl& server) : _server(server) {
  worker_ = std::thread(&DbService::WorkerThread, this);
}

DbService::~DbService() {
  running_ = false;
  worker_.join();
}

DbService::Status DbService::Initialize() {
  maindb_ = std::make_unique<sync::storage::MainDb>();

  // mount main DB
  if (!maindb_->Initialize((GetStoragePath() / "noda.db").u8string()))
    return Status::HiveError;

  LOG_INFO("Successfully initialized RootDB");
  return Status::Success;
}

void DbService::SendCommand(sync::cid_t source, std::unique_ptr<uint8_t[]> &data) {
  auto* item = s_Pool.allocate();
  item->data = std::move(data);
  item->source = source;
  item->key.next = nullptr;

  queue_.push(&item->key);
}

void DbService::ProcessTask(Tasklet& task) {
  const auto* message =
      protocol::GetMessageRoot(static_cast<const void*>(task.data.get()));

  switch (message->msg_type()) {
    case protocol::MsgType_CreateWorkspace:
      CreateWorkspace(message);
      break;
    case protocol::MsgType_CreateProject:
      CreateProject(message);
      break;
  }
}

void DbService::WorkerThread() {
  base::SetCurrentThreadPriority(base::ThreadPriority::High);
  base::SetCurrentThreadName("[DbThread]");

  while (running_) {
    while (auto* item = queue_.pop(&Tasklet::key)) {
      // not thread safe at all...
      userptr_t sender = _server.Registry().UserById(item->source);
      if (!sender)
        return;

      ProcessTask(*item);
      s_Pool.destruct(item);
    }

    std::this_thread::sleep_for(1ms);
  }
}

void DbService::CreateProject(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateProject();
  //_mainDb.CreateProject();
}

void DbService::CreateWorkspace(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateWorkspace();
  //_mainDb.CreateWorkspace(m->name()->str(), m->desc()->str());
}
}  // namespace sync_server