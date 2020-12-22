// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "Plugin.h"
#include "Pch.h"
#include "utils/Logger.h"

#include <QSettings>

namespace noda {

// plugin desc for "run" dialog
static const char kPluginComment[] = "Nomad Ida Plugin";

// "unique" plugin identifier
static const char kPluginName[] = "NODAForIDA";

// hotkey for the "run" dialog
static const char kPluginHotkey[] = "Alt-L";

Plugin::Plugin() : _syncController(), _uiController(_syncController) {
  LOG_INFO("Loaded NODA, version " GIT_BRANCH "@" GIT_COMMIT
           " Copyright(c) NOMAD Group <nomad-group.net>.");

  _client.RegisterDelegate(&_syncController);
  _client.RegisterDelegate(&_uiController);
}

Plugin::~Plugin() {}

bool Plugin::NetworkConnect() {
  QSettings settings;
  int port =
      settings.value("Nd_SyncPort", network::constants::kServerPort).toInt();
  auto addr =
      settings.value("Nd_SyncIp", network::constants::kServerIp).toString();

  bool result =
      _client.Connect(addr.toUtf8().data(), static_cast<int16_t>(port));

  if (result)
    LOG_INFO("Connected to {}:{}", addr.toUtf8().data(), port);
  else
    LOG_ERROR("Failed to connect to {}:{}", addr.toUtf8().data(), port);

  return result;
}

void Plugin::NetworkDisconnect() {
  _client.Disconnect();
}

bool Plugin::Init() {
  return true;
}

void Plugin::Run() {
  if (!auto_is_ok()) {
    LOG_ERROR("Must wait for Autoanalysis to finish before running.");
    return;
  }

  _uiController.OpenRunDialog();
}
}  // namespace noda

namespace {
using namespace noda;

Plugin* g_Plugin = nullptr;

int idaapi PluginInit() {
  g_Plugin = new Plugin();

  return g_Plugin->Init() ? PLUGIN_KEEP : PLUGIN_SKIP;
}

void idaapi PluginTerm() {
  if (g_Plugin)
    delete g_Plugin;
}

bool idaapi PluginRun(size_t arg) {
  g_Plugin->Run();
  return true;
}
}  // namespace

plugin_t PLUGIN = {
    IDP_INTERFACE_VERSION,

    // Note by Force 07/10/20:
    // We mark the Plugin with the 'FIX' attribute, so it only gets unloaded
    // when IDA closes. This is done to prevent crashes caused by QT moc'd code
    // optimizing string refs into the .rdata section of the DLL, so when the
    // plugin gets unloaded while IDA is still running it brings down the entire
    // application.
    PLUGIN_FIX, PluginInit, PluginTerm, PluginRun, kPluginComment,
    kPluginComment, kPluginName, kPluginHotkey};