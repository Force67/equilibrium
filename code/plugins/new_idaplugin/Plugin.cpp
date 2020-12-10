// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "Pch.h"
#include "Plugin.h"
#include "utils/Logger.h"

#include <QSettings>

#include <utils/Opt.h>

namespace {
  // plugin desc for "run" dialog
  constexpr char kPluginComment[] = "Nomad Ida Plugin";

  // "unique" plugin identifier
  constexpr char kPluginName[] = "NODAForIDA";

  // hotkey for the "run" dialog
  constexpr char kPluginHotkey[] = "Ctrl-Y";
} // namespace

Plugin::Plugin() :
    _session(*this),
    _shell(*this)
{
  LOG_INFO("Loaded NODA, version " GIT_BRANCH "@" GIT_COMMIT " Copyright(c) NOMAD Group <nomad-group.net>.");

  _client.RegisterComponent(&_session);
}

Plugin::~Plugin()
{
}

bool Plugin::CreateSyncSession()
{
  const bool result = _client.Start();

  if(result) {
	_session.LoginUser();
  }

  return result;
}

void Plugin::ShutdownSyncSession()
{
  _client.Stop();
}

bool Plugin::Init()
{
  return true;
}

void Plugin::Run()
{
  if(!auto_is_ok()) {
	LOG_ERROR("Must wait for Autoanalysis to finish before running.");
	return;
  }

  _shell.RunFeature();
}

namespace {
  using namespace noda;

  Plugin *g_Plugin = nullptr;

  int idaapi PluginInit()
  {
	const size_t optCount = utils::OptRegistry::Init();
	LOG_TRACE("Registered {} opts", optCount);

	g_Plugin = new Plugin();
	return g_Plugin->Init() ? PLUGIN_KEEP : PLUGIN_SKIP;
  }

  void idaapi PluginTerm()
  {
	if(g_Plugin)
	  delete g_Plugin;
  }

  bool idaapi PluginRun(size_t arg)
  {
	if(arg != -1)
	  g_Plugin->Run();

	return true;
  }
} // namespace

plugin_t PLUGIN = {
  IDP_INTERFACE_VERSION,

  // Note by Force 07/10/20:
  // We mark the Plugin with the 'FIX' attribute, so it only gets unloaded when IDA closes.
  // This is done to prevent crashes caused by QT moc'd code optimizing string refs into the
  // .rdata section of the DLL, so when the plugin gets unloaded while IDA is still running
  // it brings down the entire application.
  PLUGIN_FIX,
  PluginInit,
  PluginTerm,
  PluginRun,
  kPluginComment,
  kPluginComment,
  kPluginName,
  kPluginHotkey
};