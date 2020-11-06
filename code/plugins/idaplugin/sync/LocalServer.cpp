// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "LocalServer.h"
#include "utils/Logger.h"

namespace noda {

  LocalServer::LocalServer() :
      noda::Server(4523)
  {
  }

  LocalServer::~LocalServer()
  {
	if(_run)
	  Stop();
  }

  bool LocalServer::Start()
  {
	auto result = Server::Initialize(false);
	if(result != noda::ServerStatus::Success) {
	  LOG_ERROR("Failed to initialize the server instance (status: {})\n",
	            static_cast<int>(result));
	  return false;
	}

	_run = true;
	QThread::start();
	return true;
  }

  void LocalServer::Stop()
  {
	_run = false;
	QThread::wait();
  }

  void LocalServer::run()
  {
	while(_run) {
	  Server::Update();

	  QThread::msleep(1);
	}
  }
} // namespace noda