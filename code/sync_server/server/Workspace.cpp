// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "Workspace.h"

namespace fs = std::filesystem;

namespace noda {

  bool MountProjects(workspace_t &wks)
  {
	try {
	  auto path = fs::current_path() / "storage";
	  if(!fs::exists(path))
		fs::create_directory(path);

	  for(auto &entry : fs::directory_iterator(path)) {
		if(entry.path().extension() != ".nwks")
		  break;
	  }
	}
	catch(std::exception &) {
	  return false;
	}

	return true;
  }

  // we need to escape the workspace name on clientside via regex !!!
  bool CreateWorkspace(workspace_t &wks, const std::string &name)
  {
	auto &project = wks.emplace_back();

	return true;
  }
} // namespace noda