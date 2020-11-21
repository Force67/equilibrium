// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// stupid windows
#undef GetMessage
#undef GetMessageW

#include <filesystem>

#include "ServerImpl.h"
#include "DataHandler.h"

#include "utility/Thread.h"
#include "utils/Logger.h"

#include "protocol/generated/Message_generated.h"

using namespace std::chrono_literals;

namespace fs = std::filesystem;

namespace noda {

  static const fs::path &GetStoragePath() noexcept
  {
	static fs::path s_path{};
	if(s_path.empty()) {
	  s_path = fs::current_path() / "storage";
	  if(!fs::exists(s_path))
		fs::create_directory(s_path);
	}

	return s_path;
  }

  DataHandler::DataHandler(ServerImpl &server) :
      _server(server)
  {
	_workerThread = std::thread(&DataHandler::WorkerThread, this);
  }

  DataHandler::~DataHandler()
  {
	_run = false;
	_workerThread.join();
  }

  DataHandler::Status DataHandler::Initialize()
  {
	// mount main DB
	if(!_mainDb.Initialize((GetStoragePath() / "noda.db").u8string()))
	  return Status::HiveError;

	LOG_INFO("Successfully initialized RootDB");
	return Status::Success;
  }

  void DataHandler::QueueTask(network::connectid_t cid, const uint8_t *data, size_t size)
  {
	Task *item = _taskPool.allocate();
	item->data = std::make_unique<uint8_t[]>(size);
	item->id = cid;

	std::memcpy(item->data.get(), data, size);

	_taskQueue.push(&item->key);
  }

  void DataHandler::ProcessTask(Task &task)
  {
	const auto *message = protocol::GetMessage(static_cast<const void *>(task.data.get()));

	switch(message->msg_type()) {
	case protocol::MsgType_CreateBucket:
	  CreateBucket(message);
	  break;
	case protocol::MsgType_RemoveBucket:
	  DeleteBucket(message);
	  break;
	case protocol::MsgType_OpenNodaDB:
	  // OpenNodaDb(*sender, message);
	  break;
	}
  }

  void DataHandler::WorkerThread()
  {
	utility::SetCurrentThreadPriority(utility::ThreadPriority::High);
	utility::SetCurrentThreadName("[WorkerThread]");

	while(_run) {
	  while(auto *item = _taskQueue.pop(&Task::key)) {
		// not thread safe at all...
		userptr_t sender = _server.Registry().UserById(item->id);
		if(!sender)
		  return;

		ProcessTask(*item);
		_taskPool.destruct(item);
	  }

	  std::this_thread::sleep_for(1ms);
	}
  }

  void DataHandler::SendActionResult(protocol::MsgType type, bool failed)
  {
  }

  void DataHandler::CreateBucket(const protocol::Message *message)
  {
	// confirm or deny, perms:

	auto *msg = message->msg_as_CreateBucket();
	//bool res = _storage.AddBucket(msg->name()->str());
  }

  void DataHandler::DeleteBucket(const protocol::Message *message)
  {
	// todo: report result:

	auto *msg = message->msg_as_RemoveBucket();
	//bool res = _storage.RemBucket(msg->name()->str());
  }

  /*void DataHandler::OpenNodaDb(const NdUser &sender, const protocol::Message *message)
  {
	// TODO: respond with a list of workspaces + projects
	auto *msg = message->msg_as_OpenNodaDB();

	auto name = msg->name()->str();

	auto inst = dbref_.emplace_back(_storage, name);
	inst.AddRef();
	inst.Open();
  }*/

} // namespace noda