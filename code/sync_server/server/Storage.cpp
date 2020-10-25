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
	  res = _db.ExecuteOnly("CREATE TABLE workspaces ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT NOT NULL,"
		"created TIMESTAMP DEFAULT CURRENT_TIMESTAMP)"
			
		"CREATE TABLE projects("
		"id PRIMARY KEY,"
		"prj_name TEXT NOT NULL,"
		"prj_guid TEXT NOT NULL)");
	}

	return res;
  }

  bool Storage::AddWorkspace(const std::string &name)
  {
	database::SqliteStatement command(_db, "INSERT INTO workspaces (name) VALUES(?)");
	if(!command.Good())
	  return false;

	command.Bind(name);
	return command.Run();
  }

  // TODO: add flag for if all projects should be deleted too
  bool Storage::RemoveWorkspace(const std::string &name)
  {
	database::SqliteStatement wksQuery(_db, 
		"SELECT id FROM workspaces WHERE name = ?");
	wksQuery.Bind(name);
	if(!wksQuery.Good())
	  return false;

	wksQuery.Step();

	const int wksId = wksQuery.ColumnInt(0);

	// TODO: enumerate projects... and delete them tooo...

	database::SqliteStatement command(_db,
	    "DELETE FROM projects WHERE id = ?;"
	    "DELETE FROM workspaces WHERE id = ?;");
	command.Bind(wksId);
	command.Bind(wksId);

	return command.Run();
  }
} // namespace noda