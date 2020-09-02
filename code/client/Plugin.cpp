// NODA: Copyright(c) NOMAD Group<nomad - group.net>

#include <Plugin.h>

NodaPlugin *g_Plugin{nullptr};

NodaPlugin::NodaPlugin() {

  // bind the hooks
  hook_to_notification_point(hook_type_t::HT_UI, UiHandler, this);
}

NodaPlugin::~NodaPlugin() {
}

ssize_t NodaPlugin::UiHandler(void *pUserp, int notificationCode, va_list va) {
  if (notificationCode == ui_notification_t::ui_ready_to_run) {
  }

  return 0;
}

namespace {

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