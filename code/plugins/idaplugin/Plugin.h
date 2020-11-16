// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/UiController.h"
#include "sync/SyncController.h"

namespace noda {
  class Plugin {
  public:
	Plugin();
	~Plugin();

	bool Init();

	void Run();

	static const char kPluginComment[];
	static const char kPluginName[];
	static const char kPluginHotkey[];

  private:
	SyncController _syncController;
	UiController _uiController;
  };
} // namespace noda