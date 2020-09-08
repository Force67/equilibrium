// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <Plugin.h>
#include "IdaInc.h"

NodaPlugin::NodaPlugin() :
	_syncController(_client),
	_uiController(_client) {

  msg("Loaded NODA Copyright(c) NOMAD Group<nomad-group.net>");
}

NodaPlugin::~NodaPlugin() {
}

namespace {
  NodaPlugin *g_Plugin{ nullptr };

  const char kPluginComment[] = "Nomad Ida Plugin";
  const char kPluginName[] = "NODA";
  const char kPluginHotkey[] = "Alt-L";

  int idaapi PluginInit() {
	g_Plugin = new NodaPlugin();
	return PLUGIN_KEEP;
  }

  void idaapi PluginShutdown() {
	if (g_Plugin)
	  delete g_Plugin;
  }

  bool idaapi PluginUpdate(size_t arg) {
	return true;
  }
} // namespace

plugin_t PLUGIN = {
  IDP_INTERFACE_VERSION,
  0, // Flags
  PluginInit,
  PluginShutdown,
  PluginUpdate,
  kPluginComment,
  kPluginComment,
  kPluginName,
  kPluginHotkey
};