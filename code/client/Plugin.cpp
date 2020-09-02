// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include <Plugin.h>
#include "IdaInc.h"

NodaPlugin *g_Plugin{nullptr};

NodaPlugin::NodaPlugin() {

  // bind the hooks

  msg("Loaded Noda Copyright(c) NOMAD Group<nomad-group.net>");
}

NodaPlugin::~NodaPlugin() {
  
}

namespace {

const char kPluginComment[] = "Nomad Ida Plugin";
const char kPluginName[] = "NODA";
const char kPluginHotkey[] = "Alt-L";

int idaapi PluginInit() {
#if 0
  // we need qt
  if (!is_idaq())
    return PLUGIN_SKIP;
#endif

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
}

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