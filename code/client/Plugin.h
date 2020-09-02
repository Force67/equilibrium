// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <ui/UiController.h>
#include <sync/SyncController.h>
#include <sync/SyncClient.h>

class NodaPlugin {
public:
  NodaPlugin();
  ~NodaPlugin();

private:
  UiController _uiController;
  SyncController _syncController;
  SyncClient _client;
};

extern NodaPlugin *g_Plugin;