// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include <memory>
#include <vector>

#include <database/Project.h>

namespace noda {
using workspace_t = std::vector<database::Project>;


  void MountProjects(workspace_t &);
}