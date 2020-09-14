// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <ui/UiController.h>
#include <sync/SyncController.h>

namespace noda
{
  class Plugin
  {
  public:
	Plugin();
	~Plugin();

  private:
	UiController _uiController;
	sync::SyncController _syncController;
  };
} // namespace noda