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

void CreateMainDb(database::SqliteDB &db)
{
  bool res = db.Execute(
      R"(CREATE TABLE workspaces(
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		name TEXT NOT NULL,
		created TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
			
		CREATE TABLE projects(
		id PRIMARY KEY,
		prj_name TEXT NOT NULL,
		prj_guid TEXT NOT NULL);)");

  assert(res);
}

void InsertWorkspace(database::SqliteDB &db)
{
  //TODO: turn into prepared statements
  bool res = db.Execute(
      R"(INSERT INTO workspaces (id, name) VALUES(1, "TESTNAME");
		 INSERT INTO projects (id, prj_name, prj_guid) VALUES(1, "MyProjectName", "48fac413577bee5e05429a1aeda48b52");)");

  assert(res);
}

void RemoveWorkspace(database::SqliteDB &db)
{
  bool r = db.Execute("DELETE FROM workspaces WHERE id = 1; DELETE FROM projects WHERE id = 1");
  assert(r);
}

void CreateProject(database::SqliteDB &db, const fs::path &path, const std::string &name)
{
  assert(name.length() <= kMaxProjectNameSize);

  auto absPath = path / name;

  // in this special case we use the name as alias too!
  bool res = db.Attach(absPath.u8string(), name.c_str());
  assert(res);

  // to think about: file table...
  // TODO: think about user permission storage...
  // Workspace creator should have access to _everything_
  // Project creator is admin over his own project
  // Both can assign within their relative scope.

  // create database
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

void ListWorkspaces(database::SqliteDB &db)
{
  auto statement = fmt::format("SELECT id,name FROM workspaces");

  database::SqliteStatement query(db, statement.c_str());
  assert(query.Good());

  while(query.Step()) {
	auto text = fmt::format("{}:{}", query.ColumnInt(0), query.ColumnStr(1));

	std::puts(text.c_str());
  }
}

void RunAllTests(const fs::path &path, database::SqliteDB &db)
{
  CreateMainDb(db);
  InsertWorkspace(db);
  //RemoveWorkspace(db);

  for(int i = 0; i < 10; i++) {
	const auto name = fmt::format("project_{}", i);
	CreateProject(db, path, name);
  }

  for(int i = 0; i < 10; i++) {
	const auto name = fmt::format("project_{}", i);
	DumpAllUpdates(db, name);
  }

  ListWorkspaces(db);
  /*for (int i = 0; i < 10; i++) {
	  auto dbName = fmt::format("project_{}", i);
	  DeleteProject(db, dbName);
	}*/
}