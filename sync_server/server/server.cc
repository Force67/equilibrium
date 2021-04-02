// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "server_impl.h"
#include <base/logging.h>

namespace sync_server {
Server::Server(int16_t port) : _impl{std::make_unique<ServerImpl>(port)} {}

Server::~Server() = default;

Server::ResultStatus Server::Initialize(bool enabledStorage) {
  return _impl->Initialize(enabledStorage);
}

void Server::Update() {
  _impl->Update();
}

bool Server::IsListening() const {
  return _impl->running_;
}

int16_t Server::GetPort() const {
  return _impl->server_.Port();
}

void Server::SetLogCallback(base::LogHandler cb) {
  base::InitLogging(cb);
}
}  // namespace sync_server