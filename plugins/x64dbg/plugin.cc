// Copyright (C) Force67 2021 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

// Must include windows.h before _plugins as it pulls in dbghelp which breaks
// otherwise
#include <Windows.h>
#include <_plugins.h>
#include <base/export.h>

#include "plugin.h"

namespace {
Plugin* plugin_instance{nullptr};

// registered version
constexpr int kPluginVersion = 1;

// This is the preferred name of the plugin module in the menu system
constexpr char kWantedPluginName[] = "RETKFor64Dbg";
}  // namespace

Plugin::Plugin(int handle) : handle_(handle) {}

extern "C" {
BASE_EXPORT bool pluginit(PLUG_INITSTRUCT* init_desc) {
  init_desc->pluginVersion = kPluginVersion;
  init_desc->sdkVersion = PLUG_SDKVERSION;
  strcpy_s(init_desc->pluginName, kWantedPluginName);

  plugin_instance = new Plugin(init_desc->pluginHandle);
  return true;
}
BASE_EXPORT void plugsetup(PLUG_SETUPSTRUCT* setupStruct) {}

BASE_EXPORT bool plugstop() {
  delete plugin_instance;
  return true;
}
}
