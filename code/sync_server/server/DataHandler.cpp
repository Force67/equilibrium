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
		case protocol::MsgType_CreateWorkspace:
		  CreateWks(message);
		  break;
		case protocol::MsgType_RemoveWorkspace:
		  DeleteWks(message);
		  break;
		case protocol::MsgType_OpenProject:
		  OpenProject(*sender, message);
		  break;
		}

		packetPool.destruct(item);
	  }
	}
  }

  void DataHandler::SendActionResult(protocol::MsgType type, bool failed)
  {
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
	bool res = _storage.RemoveWorkspace(
	    msg->name()->str(),
	    msg->withProjects());
  }

  void DataHandler::OpenProject(const NdUser &sender, const protocol::Message *message)
  {
	// TODO: respond with a list of workspaces + projects
	auto *msg = message->msg_as_OpenProject();

	bool created = false;
	bool res = _storage.MakeProject(msg->name()->str(), msg->md5Hash()->str(), created);

	if(created) {
	  //sender->perms = UserPerms::Admin;
	}
  }

} // namespace noda