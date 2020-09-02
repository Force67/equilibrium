// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <ui/UiController.h>

class NodaPlugin {
public:
  NodaPlugin();
  ~NodaPlugin();

private:
  UiController _uiController;
};

extern NodaPlugin *g_Plugin;