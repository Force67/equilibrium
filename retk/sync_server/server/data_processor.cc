// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "data_processor.h"
#include "server_impl.h"

#include <base/thread.h>

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

struct DataProcessor::Tasklet {
  network::PeerId source;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Tasklet> key;
};

static base::object_pool<DataProcessor::Tasklet> s_Pool;

DataProcessor::DataProcessor(ServerImpl& server) : _server(server) {
  worker_ = std::thread(&DataProcessor::WorkerThread, this);
}

DataProcessor::~DataProcessor() {
  running_ = false;
  worker_.join();
}

DataProcessor::Status DataProcessor::Initialize() {
  maindb_ = std::make_unique<sync::storage::MainDb>();

  // TODO: storage_provider.cc

  // mount main DB
  if (!maindb_->Initialize((GetStoragePath() / "noda.db").u8string()))
    return Status::HiveError;

  // initialize it to something

  //LOG_INFO("Successfully initialized RootDB");
  return Status::Success;
}

void DataProcessor::UploadMessage(network::PeerId pid,
                              const protocol::MessageRoot*,
                              size_t len) {
  auto* tasklet = s_Pool.allocate();
  tasklet->key.next = nullptr;
  tasklet->data = std::make_unique<uint8_t[]>(len);
  tasklet->source = pid;
  queue_.push(&tasklet->key);
}

void DataProcessor::ProcessTask(Tasklet& task) {
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

void DataProcessor::WorkerThread() {
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

void DataProcessor::CreateProject(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateProject();
  //_mainDb.CreateProject();
}

void DataProcessor::CreateWorkspace(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateWorkspace();
  //_mainDb.CreateWorkspace(m->name()->str(), m->desc()->str());
}
}  // namespace sync_server