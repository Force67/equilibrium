// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Server.h"

namespace noda {

Server::Server(int argc, char **) {

}

Server::Server() {

}

Server::Status Server::Initialize() {

}

Server::Status Server::InitializeNetThread() {

}

void Server::Tick() {

}

bool Server::IsListening() const {
  return _isListening;
}
}