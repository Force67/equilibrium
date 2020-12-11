// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "sync/SyncSession.h"
#include "net/NetClient.h"
#include "ui/UiShell.h"

class Plugin final : public QObject {
  Q_OBJECT;

public:
  Plugin();
  ~Plugin();

  bool Init();
  void Run();

  bool ToggleNet();

  auto &client()
  {
	return _client;
  }

private:
  NetClient _client;
  SyncSession _session;
  UiShell _shell;
};