// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Storage.h"
#include "NdUser.h"
#include "database/SqliteStatement.h"

namespace noda {

  constexpr int kMaxProjectNameSize = 32;

  static const fs::path &GetStorageDir() noexcept
  {
	static fs::path s_path{};
	if(s_path.empty()) {
	  s_path = fs::current_path() / "storage";
	  if(!fs::exists(s_path))
		fs::create_directory(s_path);
	}

	return s_path;
  }

  NodaDb::NodaDb(Storage &st, std::string &name) :
      _parent(st),
      _name(name)
  {
  }

  bool NodaDb::Open()
  {
	auto fullPath = GetStorageDir() / _name;
	bool notCreated = !fs::exists(fullPath);

	if(!_parent._db.Attach(fullPath.u8string(), _name.c_str()))
	  return false;

	if(notCreated) {
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
	      _name, _name);

	  if(!_parent._db.Execute(statement.c_str())) {
		_parent._db.Deatch(_name.c_str());
		return false;
	  }
	}

	return true;
  }

  bool NodaDb::AddUser(NdUser &user, Perms perms /*tbd*/)
  {
	// what if user does exist already?
	database::SqliteStatement s(_parent._db, "INSERT INTO users(guid, name) VALUES(?,?);");
	if(!s.Good())
	  return false;

	s.Bind(user.Guid());
	s.Bind(user.Name());
	return s.Run();
  }

  Storage::Storage()
  {
  }

  bool Storage::Initialize(const StorageConfig &config)
  {
	fs::path path = GetStorageDir() / "hive";
	bool create = !fs::exists(path);

	if(!_db.open(path.u8string()))
	  return false;

	if(create) {
	  bool res = _db.Execute(
	      "CREATE TABLE buckets("
	      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      "name TEXT NOT NULL);"

	      "CREATE TABLE file_root("
	      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
	      "prj_id INTEGER,"
	      "type INTEGER," // reserved for future use
	      "name TEXT NOT NULL,"
	      "path TEXT NOT NULL);"); // initial hive layout

	  return res;
	}

	return true;
  }

  int Storage::BucketByName(const std::string &name)
  {
	database::SqliteStatement s(_db, "SELECT id FROM buckets WHERE name = ?");
	if(s.Good()) {
	  s.Bind(name);

	  if(s.Step()) {
		return s.ColumnInt(0);
	  }
	}

	return -1;
  }

  bool Storage::AddBucket(const std::string &name)
  {
	database::SqliteStatement command(_db, "INSERT INTO buckets (name) VALUES(?)");
	if(!command.Good())
	  return false;

	bool res = command.Bind(name);
	return res && command.Run();
  }

  bool Storage::RemBucket(const std::string &name)
  {
	const int id = BucketByName(name);

	database::SqliteStatement command(_db,
	                                  "DELETE FROM buckets WHERE id = ?;"
	                                  "DELETE FROM file_root WHERE prj_id = ?;");
	command.Bind(id);
	command.Bind(id);

	return command.Run();
  }
} // namespace noda