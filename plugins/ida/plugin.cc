// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include <QSettings>
#include "plugin.h"
#include "bindings/binding.h"

#include "utils/opt.h"
#include "utils/ida_log_impl.h"

namespace {
Plugin* g_Plugin = nullptr;

constexpr char kPluginComment[] = "Reverse Engineering Toolkit for IDA Pro";

// This is the preferred name of the plugin module in the menu system
// The preferred name may be overridden in plugins.cfg file
constexpr char kWantedPluginName[] = "RETKForIDA";

// this will bring up the quick menu
constexpr char kWantedPluginHotkey[] = "Ctrl-Y";
}  // namespace

Plugin::Plugin() : sync_(*this), ui_(*this), work_bench_(*this) {
  BindingBase::BindAll();
  LOG_INFO("Loaded RETK, version " GIT_BRANCH "@" GIT_COMMIT
           " Created by Force67 <github.com/Force67>.");
}

Plugin::~Plugin() {
  utils::OptRegistry::Save();
  BindingBase::UnBindAll();
}

bool Plugin::SyncToggle() {
    #if 0
  if (!auto_is_ok() && !sync_.IsOnline()) {
    LOG_ERROR("Must wait for Autoanalysis to finish before connecting.");
    return false;
  }

  if (sync_.Connected()) {
    LOG_TRACE("Plugin::ToggleNet() -> Disconnect");

    // TBD.
    sync_.Stop();
    return true;
  }

  bool result = sync_.Start();
  LOG_TRACE("Plugin::ToggleNet() -> Connect {}", result);
  return result;
  #endif
  return true;
}

bool Plugin::Init() {
  return true;
}

bool Plugin::run(size_t arg) {
  // This is the menu available under the default IDA plugins dropdown menu.
  if (ial::IDB::IsBusy()) {
    LOG_ERROR("IDA is busy. Must wait for Autoanalysis to finish before running.");
    return false;
  }

  ui_.RunFeature();
  return true;
}

plugmod_t* Plugin::Create() {
  InitIdaLogHandler();
  // opts must be created before any other component
  // to avoid ending up with invalid references.
  utils::OptRegistry::Load();

  g_Plugin = new Plugin();
  return g_Plugin->Init() ? g_Plugin : nullptr;
}

const plugin_t& GetPluginDesc() {
  return PLUGIN;
}

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,

    // Note by Force 07/10/20:
    // We mark the Plugin with the 'FIX' attribute, so it only gets unloaded
    // when IDA closes. This is done to prevent crashes caused by QT moc'd code
    // optimizing string refs into the .rdata section of the DLL, so when the
    // plugin gets unloaded while IDA is still running it brings down the entire
    // application.
    PLUGIN_FIX |       // < plugin is pinned
        PLUGIN_MULTI,  // < plugin works with multiple idbs
    Plugin::Create, nullptr, nullptr, kPluginComment, kPluginComment,
    kWantedPluginName, kWantedPluginHotkey};