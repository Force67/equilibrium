// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "ServerImpl.h"
#include "utils/Logger.h"

namespace noda {
  Server::Server(int16_t port) :
      _impl{ std::make_unique<ServerImpl>(port) }
  {
  }

  Server::~Server() = default;

  ServerStatus Server::Initialize(bool enabledStorage)
  {
	return _impl->Initialize(enabledStorage);
  }

  void Server::Update()
  {
	_impl->Update();
  }

  bool Server::IsListening() const
  {
	return _impl->_listening;
  }

  int16_t Server::GetPort() const
  {
	return _impl->_server.Port();
  }

  void Server::SetLogCallback(logcallback_t cb)
  {
	utils::SetLogCallback(cb);
  }
} // namespace noda