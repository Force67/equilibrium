// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Storage.h"
#include "NdUser.h"
#include "database/SqliteStatement.h"

namespace noda {

  constexpr int kMaxProjectNameSize = 32;

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

  bool Storage::Initialize(const StorageConfig &config)
  {
	auto path = GetStorageDir() / "hive";
	bool create = !fs::exists(path);

	const auto u8Str = path.u8string();
	if(!_db.open(u8Str.c_str()))
	  return false;

	bool res = true;
	if(create) {
	  res = _db.Execute(
	      "CREATE TABLE workspaces ("
	      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      "name TEXT NOT NULL,"
	      "created TIMESTAMP DEFAULT CURRENT_TIMESTAMP);"

	      "CREATE TABLE projects("
	      "id PRIMARY KEY,"
	      "prj_name TEXT NOT NULL,"
	      "prj_guid TEXT NOT NULL);");
	}

	return res;
  }

  // TODO: with project?
  bool Storage::AddWorkspace(const std::string &name)
  {
	database::SqliteStatement command(_db, "INSERT INTO workspaces (name) VALUES(?)");
	if(!command.Good())
	  return false;

	command.Bind(name);
	return command.Run();
  }

  bool Storage::RemoveWorkspace(const std::string &name, bool withProjects)
  {
	database::SqliteStatement wksQuery(_db,
	                                   "SELECT id FROM workspaces WHERE name = ?");
	wksQuery.Bind(name);
	if(!wksQuery.Good())
	  return false;

	// todo: handle > 0 results
	wksQuery.Step();

	const int wksId = wksQuery.ColumnInt(0);

	// TODO: enumerate projects... and delete them tooo...
	if(withProjects) {
	}

	database::SqliteStatement command(_db,
	                                  "DELETE FROM projects WHERE id = ?;"
	                                  "DELETE FROM workspaces WHERE id = ?;");
	command.Bind(wksId);
	command.Bind(wksId);

	return command.Run();
  }

  bool Storage::MakeProject(
      const std::string &name,
      const std::string &md5,
      bool &created)
  {
	if(name.length() > kMaxProjectNameSize)
	  return false;

	auto fullPath = GetStorageDir() / name;
	created = !fs::exists(fullPath);

	if(!_db.Attach(fullPath.u8string(), name.c_str()))
	  return false;

	if(created) {
	  auto statement = fmt::format(
	      R"(CREATE TABLE {}.updates(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id TEXT NOT NULL,
			msg_type BIGINT,
			data BLOB);

			CREATE TABLE {}.users(
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			guid TEXT NOT NULL,
			name TEXT NOT NULL);)",
	      name, name);

	  if(!_db.ExecuteOnly(statement.c_str())) {
		_db.Deatch(name.c_str());
		return false;
	  }
	}

	return true;
  }

  bool Storage::AddUser(const std::string &, const NdUser &user)
  {
	return false;
  }
} // namespace noda