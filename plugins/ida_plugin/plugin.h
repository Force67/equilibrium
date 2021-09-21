// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "idb/idb_store.h"
#include "ui/plugin_ui.h"
#include "sync/ida_sync.h"
#include "tools/workbench.h"

class Plugin final : public QObject, public plugmod_t {
  Q_OBJECT;

 public:
  Plugin();
  ~Plugin() override;

  static plugmod_t* Create();

  bool Init(); 
  // plugmod_t
  bool run(size_t arg) override;

  bool SyncToggle();

  inline IdaSync& sync() { return sync_; }
  inline PluginUi& ui() { return ui_; }
  //inline auto& Tools() { return tools_; }

 private:
  IdbStore store_;
  IdaSync sync_;
  PluginUi ui_;
  Workbench work_bench_;
};

const plugin_t& GetPluginDesc();