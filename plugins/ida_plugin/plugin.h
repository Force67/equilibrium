// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "storage/data_store.h"
#include "ui/plugin_ui.h"
#include "sync/ida_sync.h"
#include "tools/toolbox.h"

class Plugin final : public QObject, public plugmod_t {
  Q_OBJECT;

 public:
  Plugin();
  ~Plugin() override;

  static plugmod_t* Create();

  bool Init(); 
  bool run(size_t arg);

  bool SyncToggle();

  inline auto& Sync() { return sync_; }
  inline auto& Ui() { return ui_; }
  inline auto& Tools() { return tools_; }

 private:
  DataStore store_;
  IdaSync sync_;
  PluginUi ui_;
  tools::Toolbox tools_;
};