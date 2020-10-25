// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Storage.h"
#include "database/SqliteStatement.h"

namespace noda {

  Storage::Storage()
  {
  }

  const fs::path &Storage::GetStorageDir() noexcept
  {
	static fs::path s_path{};
	if(s_path.empty()) {
	  s_path = fs::current_path() / "storage";
	  if(!fs::exists(s_path))
		fs::create_directory(s_path);
	}

	return s_path;
  }

  bool Storage::Initialize()
  {
	auto hivePath = GetStorageDir() / "hive";
	const bool create = fs::exists(hivePath);

	const auto u8Str = hivePath.u8string();
	if(!_db.open(u8Str.c_str()))
	  return false;

	bool res = true;
	if(create) {
	  res = _db.ExecuteOnly(
	      R"(CREATE TABLE workspaces(
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		name TEXT NOT NULL,
		created TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
			
		CREATE TABLE projects(
		id PRIMARY KEY,
		prj_name TEXT NOT NULL,
		prj_guid TEXT NOT NULL);)");
	}

	return res;
  }

  void Storage::AddWorkspace(const std::string &name)
  {
	database::SqliteStatement command(_db, "INSERT INTO workspaces (name) VALUES(?);");
	if(!command.Good()) {
	  // TODO: report error
	}

	command.Bind(name);
	if(!command.Run()) {
	  // TODO: report
	}
  }

  void Storage::RemoveWorkspace(const std::string &name)
  {
	// TODO: delete all tables?

	database::SqliteStatement command(_db, "DELETE FROM workspaces WHERE id = 1;");
  }
} // namespace noda