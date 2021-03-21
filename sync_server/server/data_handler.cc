// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "data_handler.h"
#include "server_impl.h"

#include "utility/thread.h"
#include "utils/logger.h"

#include "protocol/generated/MessageRoot_generated.h"

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

DataHandler::DataHandler(ServerImpl& server) : _server(server) {
  _workerThread = std::thread(&DataHandler::WorkerThread, this);
}

DataHandler::~DataHandler() {
  _run = false;
  _workerThread.join();
}

DataHandler::Status DataHandler::Initialize() {
  // mount main DB
  if (!_mainDb.Initialize((GetStoragePath() / "noda.db").u8string()))
    return Status::HiveError;

  LOG_INFO("Successfully initialized RootDB");
  return Status::Success;
}

void DataHandler::QueueTask(network::connectid_t cid,
                            const uint8_t* data,
                            size_t size) {
  Task* item = _taskPool.allocate();
  item->data = std::make_unique<uint8_t[]>(size);
  item->id = cid;
  item->key.next = nullptr;

  std::memcpy(item->data.get(), data, size);

  _taskQueue.push(&item->key);
}

void DataHandler::ProcessTask(Task& task) {
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

void DataHandler::WorkerThread() {
  utility::SetCurrentThreadPriority(utility::ThreadPriority::High);
  utility::SetCurrentThreadName("[WorkerThread]");

  while (_run) {
    while (auto* item = _taskQueue.pop(&Task::key)) {
      // not thread safe at all...
      userptr_t sender = _server.Registry().UserById(item->id);
      if (!sender)
        return;

      ProcessTask(*item);
      _taskPool.destruct(item);
    }

    std::this_thread::sleep_for(1ms);
  }
}

void DataHandler::CreateProject(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateProject();
  //_mainDb.CreateProject();
}

void DataHandler::CreateWorkspace(const protocol::MessageRoot* msg) {
  auto* m = msg->msg_as_CreateWorkspace();
  _mainDb.CreateWorkspace(m->name()->str(), m->desc()->str());
}
}  // namespace sync_server