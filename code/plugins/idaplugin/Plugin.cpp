// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "Plugin.h"
#include "utils/Logger.h"

namespace noda {
  Plugin::Plugin() :
      _syncController(),
      _uiController(_syncController)
  {
	LOG_INFO("Loaded NODA, version " GIT_BRANCH "@" GIT_COMMIT " Copyright(c) NOMAD Group <nomad-group.net>.");
  }

  Plugin::~Plugin()
  {
	//https://github.com/citra-emu/citra/blob/master/src/citra_qt/loading_screen.cpp
  }

  bool Plugin::Init()
  {
	return true;
  }

  void Plugin::Run()
  {
	_uiController.OpenRunDialog();
  }
} // namespace noda

namespace {
  using namespace noda;

  Plugin *g_Plugin = nullptr;

  const char kPluginComment[] = "Nomad Ida Plugin";
  const char kPluginName[] = "NODA";
  const char kPluginHotkey[] = "Alt-L";

  int idaapi PluginInit()
  {
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