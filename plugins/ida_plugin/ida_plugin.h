// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "sync/sync_client.h"
#include "sync/sync_session.h"
#include "ui/ui_shell.h"
#include "tools/toolbox.h"

#include "storage/data_store.h"

class IdaPlugin final : public QObject, public plugmod_t {
  Q_OBJECT;

 public:
  IdaPlugin();
  ~IdaPlugin() override;

  static plugmod_t* Create();

  bool Init();
  bool run(size_t arg);

  bool ToggleNet();

  inline NetClient& client() { return _client; }
  inline SyncSession& session() { return _session; }
  inline tools::Toolbox& toolbox() { return _toolbox; }

 private:
  DataStore storage_;

  NetClient _client;
  SyncSession _session;
  UiShell _shell;
  tools::Toolbox _toolbox;
};