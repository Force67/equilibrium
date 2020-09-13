// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <ui/UiController.h>
#include <sync/SyncController.h>
#include <sync/SyncClient.h>

#include "IdaInc.h"

namespace
{
  struct Plugin {
	Plugin() :
	    _syncController(_client),
	    _uiController(_client)
	{
	  msg("Loaded NODA Copyright(c) NOMAD Group<nomad-group.net>");
	}

	~Plugin()
	{
	}

	UiController _uiController;
	SyncController _syncController;
	SyncClient _client;
  } * g_Plugin;

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