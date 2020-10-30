// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "ServerImpl.h"
#include "DataHandler.h"

#include "utility/Thread.h"

namespace noda {

  inline utility::object_pool<InPacket> packetPool;

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

  void DataHandler::Queue(netlib::connectid_t cid, netlib::Packet *packet)
  {
	InPacket *item = packetPool.construct(cid, packet);
	_packetQueue.push(&item->key);
  }

  DataHandler::Status DataHandler::Initialize()
  {
	StorageConfig config;
	if(!_storage.Initialize(config))
	  return Status::HiveError;

	return Status::Success;
  }

  void DataHandler::WorkerThread()
  {
	utility::SetCurrentThreadPriority(utility::ThreadPriority::High);
	utility::SetCurrentThreadName("[WorkerThread]");

	while(_run) {
	  while(auto *item = _packetQueue.pop(&InPacket::key)) {
		const netlib::Packet &packet = item->packet;

		const auto sender = _server.UserById(item->id);
		const auto *message = protocol::GetMessage(static_cast<const void *>(packet.data()));

		switch(message->msg_type()) {
		case protocol::MsgType_CreateBucket:
		  CreateBucket(message);
		  break;
		case protocol::MsgType_RemoveBucket:
		  DeleteBucket(message);
		  break;
		case protocol::MsgType_OpenNodaDB:
		  OpenNodaDb(*sender, message);
		  break;
		}

		packetPool.destruct(item);
	  }
	}
  }

  void DataHandler::SendActionResult(protocol::MsgType type, bool failed)
  {
  }

  void DataHandler::CreateBucket(const protocol::Message *message)
  {
	// confirm or deny, perms:

	auto *msg = message->msg_as_CreateBucket();
	bool res = _storage.AddBucket(msg->name()->str());
  }

  void DataHandler::DeleteBucket(const protocol::Message *message)
  {
	  // todo: report result:

	auto *msg = message->msg_as_RemoveBucket();
	bool res = _storage.RemBucket(msg->name()->str());
  }

  void DataHandler::OpenNodaDb(const NdUser &sender, const protocol::Message *message)
  {
	// TODO: respond with a list of workspaces + projects
	auto *msg = message->msg_as_OpenNodaDB();
	
	auto name = msg->name()->str();

	auto inst = dbref_.emplace_back(_storage, name);
	inst.AddRef();
	inst.Open();
  }

} // namespace noda