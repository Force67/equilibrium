// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "db_handler.h"
#include "server_impl.h"

#include <base/thread.h>
#include "utils/server_logger.h"

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

struct DbHandler::Tasklet {
  network::connectid_t source;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Tasklet> key;
};

static base::object_pool<DbHandler::Tasklet> s_Pool;

DbHandler::DbHandler(ServerImpl& server) : _server(server) {
  worker_ = std::thread(&DbHandler::WorkerThread, this);
}

DbHandler::~DbHandler() {
  running_ = false;
  worker_.join();
}

DbHandler::Status DbHandler::Initialize() {
  // mount main DB
  if (!_mainDb.Initialize((GetStoragePath() / "noda.db").u8string()))
    return Status::HiveError;

  LOG_INFO("Successfully initialized RootDB");
  return Status::Success;
}

void DbHandler::QueueTask(network::connectid_t cid,
                            const uint8_t* data,
                            size_t size) {
  auto* item = s_Pool.allocate();
  item->data = std::make_unique<uint8_t[]>(size);
  item->source = cid;
  item->key.next = nullptr;

  std::memcpy(item->data.get(), data, size);

  queue_.push(&item->key);
}

void DbHandler::ProcessTask(Tasklet& task) {
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

void DbHandler::WorkerThread() {
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

void DbHandler::CreateProject(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateProject();
  //_mainDb.CreateProject();
}

void DbHandler::CreateWorkspace(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateWorkspace();
  //_mainDb.CreateWorkspace(m->name()->str(), m->desc()->str());
}
}  // namespace sync_server