// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <filesystem>

#include "Workspace.h"

namespace fs = std::filesystem;

#include "database/SqliteDB.h"

namespace noda {

  const fs::path &GetStorageDir() noexcept
  {
	static fs::path s_path{};
	if(s_path.empty()) {
	  s_path = fs::current_path() / "storage"; // hard-coded for now, TODO: make configurable
	  if(!fs::exists(s_path))
		fs::create_directory(s_path);
	}

	return s_path;
  }

  bool MountProjects(workspace_t &wks)
  {
	const fs::path &path = GetStorageDir();

	try {
	  for(auto &entry : fs::directory_iterator(path)) {
		if(entry.path().extension() != ".db")
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

  bool InitializeMainDB(database::SqliteDB &db)
  {
	const fs::path &path = GetStorageDir() / "main.db";
	const std::string u8Path = path.u8string();

	if(!fs::exists(path)) {
	  if(!db.create(u8Path.c_str()))
		return false;

	  db.ExecuteOnly(
	      R"(CREATE TABLE wksinfo(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT NOT NULL,
			created TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
			
			CREATE TABLE wksref(
			wks_id PRIMARY KEY,
			prj_name TEXT NOT NULL,
			prj_guid TEXT NOT NULL))");

	  return true;
	}

	return db.open(u8Path.c_str());
  }
} // namespace noda