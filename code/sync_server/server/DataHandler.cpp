// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "ServerImpl.h"
#include "DataHandler.h"

#include "utility/Thread.h"

// stupid windows
#undef GetMessage
#undef GetMessageW

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
	if(!_storage.Initialize())
	  return Status::MainDbError;

	return Status::Success;
  }

  void DataHandler::WorkerThread()
  {
	utility::SetCurrentThreadPriority(utility::ThreadPriority::High);
	utility::SetCurrentThreadName("[WorkerThread]");

	while(_run) {
	  while(auto *item = _packetQueue.pop(&InPacket::key)) {
		const netlib::Packet &packet = item->packet;

		const auto *message = protocol::GetMessage(static_cast<const void *>(packet.data()));

		HandleMessage(message);
		packetPool.destruct(item);
	  }
	}
  }

  void DataHandler::HandleMessage(const protocol::Message *message)
  {
	switch(message->msg_type()) {
	case protocol::MsgType_CreateWorkspace:
	  CreateWks(message);
	  break;
	case protocol::MsgType_RemoveWorkspace:
	  DeleteWks(message);
	  break;
	case protocol::MsgType_WorkspaceList: {
	  break;
	}
	
	}
  }

  void DataHandler::CreateWks(const protocol::Message *message)
  {
	// TODO: confirm or deny message
	// also verify user permissions...
	auto *msg = message->msg_as_CreateWorkspace();
	bool res = _storage.AddWorkspace(msg->name()->str());
  }

  void DataHandler::DeleteWks(const protocol::Message *message)
  {
	auto *msg = message->msg_as_RemoveWorkspace();
	bool res = _storage.RemoveWorkspace(msg->name()->str());
  }

  void DataHandler::ListWks(const protocol::Message *message)
  {
	  
  
  }
} // namespace noda