// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "network/TCPClient.h"
#include "sync/SyncController.h"
#include "ui/UiController.h"

namespace noda {
class Plugin {
 public:
  Plugin();
  ~Plugin();

  bool Init();

  void Run();

  bool NetworkConnect();
  void NetworkDisconnect();

  auto& client() { return _client; }

 private:
  SyncController _syncController;
  UiController _uiController;

  network::ScopedSocket _sock;
  network::TCPClient _client;
};
}  // namespace noda