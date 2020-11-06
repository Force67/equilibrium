// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "flatbuffers/flatbuffers.h"

#include "database/SqliteDB.h"
#include "database/SqliteStatement.h"

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"
#include "moc_protocol/IdaSync_generated.h"

#include <filesystem>

namespace fs = std::filesystem;

// length of md5
constexpr int kMaxProjectNameSize = 32;

/*
Storage
├───hive
│   ├───Projects
│   │   ├───files
│   │   └───userDB
│   └───Admins
│       └───TBD
├───folder1
│   └───MyProject
*/

// Example:
/*
Storage
├───CTF
│   ├───Northsec 2016
│   ├───RECON 2016
│   │   ├───file1.idb
│   │   └───file2.py
│   └───IOT
*/

// Hive needs to keep ALL files

// - hive
// - sdfhksdfosdfsdfdsf.db -> users, files

void CreateHive(database::SqliteDB &db)
{
  bool res = db.Execute(
      "CREATE TABLE projects("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "name TEXT NOT NULL);"

      // simple list of all files related to projects
      "CREATE TABLE file_root("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "prj_id INTEGER,"
      "type INTEGER," // reserved for future use
      "name TEXT NOT NULL,"
      "path TEXT NOT NULL);"); // initial hive layout

  // workspace id
  assert(res);
}

void CreateProject(database::SqliteDB &db, const fs::path &path, const std::string &name)
{
  assert(name.length() <= kMaxProjectNameSize);

  database::SqliteStatement s(db, "INSERT INTO projects(name) VALUES(?);");
  assert(s.Good());

  bool result = s.Bind(name);
  assert(result);

  assert(s.Run());
}

int GetProjectId(database::SqliteDB &db, const std::string &name)
{
  database::SqliteStatement s(db, "SELECT id FROM projects WHERE name = ?");
  assert(s.Good());

  s.Bind(name);

  if(s.Step()) {
	return s.ColumnInt(0);
  }

  return -1;
}

// project = IDB Snapshot
void CreateNodaDB(
    database::SqliteDB &db,
    const std::string &wksName,
    const fs::path &path,
    const std::string &name)
{
  int wksId = GetProjectId(db, wksName);
  assert(wksId != -1);

  auto dbPath = (path / name).u8string();

  // in this special case we use the name as alias too!
  bool res = db.Attach(dbPath, name.c_str());
  assert(res);

  {
	database::SqliteStatement s(db, "INSERT INTO file_root(prj_id, type, name, path) VALUES(?,?,?,?);");
	assert(s.Good());

	res = s.Bind(wksId);
	res = s.Bind(0);
	res = s.Bind(name);
	res = s.Bind(path.u8string());
	res = s.Run();
	assert(res);
  }

  auto sql = fmt::format(
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

  // to think about: file table...
  // TODO: think about user permission storage...
  // Workspace creator should have access to _everything_
  // Project creator is admin over his own project
  // Both can assign within their relative scope.

  // create database

  res = db.Execute(sql.c_str());
  assert(res);

  // insert a few updates
  sql = fmt::format(
      "INSERT INTO {}.updates(user_id, msg_type, data)"
      " VALUES(?, ?, ?);",
      name);

  flatbuffers::FlatBufferBuilder fbb;
  auto packet = protocol::CreateMessage(
      fbb, protocol::MsgType_sync_MakeCode,
      protocol::sync::CreateMakeCode(fbb, 0x133713371337).Union());
  fbb.Finish(packet);

  const protocol::Message *message = protocol::GetMessage(fbb.GetBufferPointer());

  for(int i = 0; i < 3; i++) {
	database::SqliteStatement insertCommand(db, sql.c_str());
	assert(insertCommand.Good());

	res = insertCommand.Bind("{1337-1337-1337}");
	res = insertCommand.Bind(message->msg_type());
	res = insertCommand.Bind(fbb.GetBufferPointer(), fbb.GetSize());
	assert(res);

	res = insertCommand.Run();
	assert(res);
  }

  auto rid = db.LastestRowId();
  //__debugbreak();
}

void DeleteProject(database::SqliteDB &hiveDb, const fs::path &path, const std::string &name)
{
  assert(name.length() <= kMaxProjectNameSize);

  bool res = hiveDb.Deatch(name.c_str());
  assert(res);

  fs::remove(path / name);
}

void DumpAllUpdates(database::SqliteDB &db, const std::string &name)
{
  auto statement = fmt::format(
      "SELECT * FROM {}.updates", name);

  database::SqliteStatement dumpCommand(db, statement.c_str());
  assert(dumpCommand.Good());

  /*
  	id INTEGER PRIMARY KEY AUTOINCREMENT,
	user_id TEXT NOT NULL,
	msg_type BIGINT,
	data BLOB);
  */

  while(dumpCommand.Step()) {
	auto string = fmt::format("{}:{}:{}:{}",
	                          dumpCommand.ColumnInt(0),
	                          dumpCommand.ColumnStr(1),
	                          dumpCommand.ColumnBigInt(2),
	                          dumpCommand.ColumnBlob(3));

	std::puts(string.c_str());
  }
}

void ListProjects(database::SqliteDB &db)
{
  auto statement = fmt::format("SELECT id,name FROM projects");

  database::SqliteStatement query(db, statement.c_str());
  assert(query.Good());

  while(query.Step()) {
	auto text = fmt::format("{}:{}", query.ColumnInt(0), query.ColumnStr(1));

	std::puts(text.c_str());
  }
}

void RunAllTests(const fs::path &path, database::SqliteDB &db)
{
  CreateHive(db);
  CreateProject(db, path, "TestProject");

  for(int i = 0; i < 10; i++) {
	const auto name = fmt::format("project_{}", i);
	CreateNodaDB(db, "TestProject", path, name);
  }

  for(int i = 0; i < 10; i++) {
	const auto name = fmt::format("project_{}", i);
	DumpAllUpdates(db, name);
  }

  ListProjects(db);
  /*for (int i = 0; i < 10; i++) {
	  auto dbName = fmt::format("project_{}", i);
	  DeleteProject(db, dbName);
	}*/
}