// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include <memory>
#include <vector>

#include <database/Project.h>
#include "database/SqliteDB.h"

namespace noda {
  using workspace_t = std::vector<database::Project>;

  // workspace stores all projects in one file.
  bool MountProjects(workspace_t &);
  bool CreateWorkspace(workspace_t &, const std::string &name);

  bool InitializeMainDB(database::SqliteDB &);
} // namespace noda