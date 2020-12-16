// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "sync/sync_client.h"
#include "sync/sync_session.h"
#include "ui/ui_shell.h"

#include "Pch.h"

class Plugin final : public QObject, public plugmod_t {
  Q_OBJECT;

 public:
  Plugin();
  ~Plugin() override;

  static plugmod_t* Create();

  bool Init();

  bool run(size_t arg);

  bool ToggleNet();

  NetClient& client();
  SyncSession& session();

 private:
  NetClient _client;
  SyncSession _session;
  UiShell _shell;
};