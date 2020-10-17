// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "Workspace.h"

namespace fs = std::filesystem;

namespace noda {

  void MountProjects(workspace_t &wks)
  {
	for(auto &entry : fs::directory_iterator("db")) {

	}
  }
} // namespace noda