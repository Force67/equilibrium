// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "Plugin.h"
#include "IdaInc.h"

namespace noda
{
  Plugin::Plugin() :
      _syncController(),
      _uiController(_syncController)
  {
	msg("Loaded NODA Copyright(c) NOMAD Group <nomad-group.net>.\n");
  }

  Plugin::~Plugin()
  {
  }
} // namespace noda

namespace
{
  using namespace noda;

  Plugin *g_Plugin = nullptr;

  const char kPluginComment[] = "Nomad Ida Plugin";
  const char kPluginName[] = "NODA";
  const char kPluginHotkey[] = "Alt-L";

  int idaapi PluginInit()
  {
	g_Plugin = new Plugin();
	return PLUGIN_KEEP;
  }

  void idaapi PluginShutdown()
  {
	if(g_Plugin)
	  delete g_Plugin;
  }

  bool idaapi PluginRun(size_t arg)
  {
	return true;
  }
} // namespace

plugin_t PLUGIN = {
  IDP_INTERFACE_VERSION,
  0, // Flags
  PluginInit,
  PluginShutdown,
  PluginRun,
  kPluginComment,
  kPluginComment,
  kPluginName,
  kPluginHotkey
};