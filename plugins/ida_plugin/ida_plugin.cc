// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "pch.h"
#include "ida_plugin.h"
#include "utils/logger.h"

#include <QSettings>

#include <utils/opt.h>

namespace {
IdaPlugin* g_Plugin = nullptr;

constexpr char kPluginComment[] = "Nomad Ida Plugin";
constexpr char kPluginName[] = "NODAForIDA";
constexpr char kPluginHotkey[] = "Ctrl-Y";
}  // namespace

IdaPlugin::IdaPlugin() : _session(*this), _shell(*this) {
  LOG_INFO("Loaded RETK, version " GIT_BRANCH "@" GIT_COMMIT
           " Created by Force67 <github.com/Force67>.");

  _client.RegisterComponent(&_session);
}

IdaPlugin::~IdaPlugin() {
  utils::OptRegistry::Save();
}

bool IdaPlugin::ToggleNet() {
  if (_client.Connected()) {
    LOG_TRACE("Plugin::ToggleNet() -> Disconnect");

    _session.LogOff();
    _client.Stop();
    return true;
  }

  bool result = _client.Start();
  LOG_TRACE("Plugin::ToggleNet() -> Connect {}", result);

  if (result) {
    _session.LoginUser();
  }

  return result;
}

bool IdaPlugin::Init() {
  return true;
}

bool IdaPlugin::run(size_t arg) {
  /*if (arg != -1) {
    LOG_TRACE("Plugin::run() -> Arg : {}", arg);
    return false;
  }*/

  if (!auto_is_ok()) {
    LOG_ERROR("Must wait for Autoanalysis to finish before running.");
    return false;
  }

  _shell.RunFeature();
  return true;
}

plugmod_t* IdaPlugin::Create() {
  const size_t optCount = utils::OptRegistry::Load();
  LOG_TRACE("Registered {} opts", optCount);

  g_Plugin = new IdaPlugin();

  return g_Plugin->Init() ? g_Plugin : nullptr;
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
    IdaPlugin::Create, 
    nullptr, nullptr, 
    kPluginComment, 
    kPluginComment, 
    kPluginName,
    kPluginHotkey};