// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
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