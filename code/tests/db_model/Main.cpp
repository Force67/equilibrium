// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>
#include <algorithm>
#include <filesystem>

#include <netlib/NetLib.h>
#include <netlib/Client.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "flatbuffers/flatbuffers.h"

#include "moc_protocol/Message_generated.h"
#include "moc_protocol/Handshake_generated.h"
#include "moc_protocol/IdaSync_generated.h"

#include "database/SqliteDB.h"
#include "database/SqliteStatement.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define MAIN_DB_NAME "hive"

// length of md5
constexpr int kMaxProjectNameSize = 32;

namespace fs = std::filesystem;

const fs::path &GetStorageDir() noexcept
{
  static fs::path s_path{};
  if(s_path.empty()) {
	s_path = fs::current_path() / "test_storage";
	if(!fs::exists(s_path))
	  fs::create_directory(s_path);
  }

  return s_path;
}

void OpenHiveDB(database::SqliteDB &db)
{
}

void CreateMainDb()
{
  database::SqliteDB db;
  if(!db.create(MAIN_DB_NAME)) {
	std::puts("CreateMainDb() failed");
	return;
  }

  db.ExecuteOnly(
      R"(CREATE TABLE workspaces(
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		name TEXT NOT NULL,
		created TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
			
		CREATE TABLE projects(
		id PRIMARY KEY,
		prj_name TEXT NOT NULL,
		prj_guid TEXT NOT NULL);)");

  db.close();
}

void InsertWorkspace()
{
  database::SqliteDB db;
  if(!db.open(MAIN_DB_NAME)) {
	std::puts("InsertWorkspace() failed!");
	return;
  }

  db.ExecuteOnly(
      R"(INSERT INTO workspaces (id, name) VALUES(1, "TESTNAME");
		 INSERT INTO projects (id, prj_name, prj_guid) VALUES(1, "MyProjectName", "48fac413577bee5e05429a1aeda48b52");)");

  db.close();
}

void RemoveWorkspace()
{
  database::SqliteDB db;
  if(!db.open(MAIN_DB_NAME)) {
	std::puts("RemoveWorkspace() failed!");
	return;
  }

  db.ExecuteOnly("DELETE FROM workspaces WHERE id = 1; DELETE FROM projects WHERE id = 1");
  db.close();
}

void CreateProject(database::SqliteDB &hiveDb, const std::string &name)
{
  assert(name.length() <= kMaxProjectNameSize);

  auto absPath = GetStorageDir() / name;

  // in this special case we use the name as alias too!
  bool res = hiveDb.Attach(absPath.u8string(), name.c_str());
  assert(res);

  // to think about: file table...

  // create database
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

  res = hiveDb.ExecuteOnly(statement.c_str());
  assert(res);

  // insert a few updates
  statement = fmt::format(
      "INSERT INTO {}.updates(user_id, msg_type, data)"
      " VALUES(?, ?, ?);",
      name);

  for(int i = 0; i < 1; i++) {
	const uint8_t blob[8] = { 1, 3, 3, 7, 1, 3, 3, 7 };
	database::SqliteStatement insertCommand(hiveDb, statement.c_str());
	assert(insertCommand.Good());

	res = insertCommand.Bind("{1337-1337-1337}");
	res = insertCommand.Bind(1337);
	res = insertCommand.Bind(&blob[0], sizeof(blob));
	assert(res);

	res = insertCommand.Run();
	assert(res);
  }
}

void DeleteProject(database::SqliteDB &hiveDb, const std::string &name)
{
  assert(name.length() <= kMaxProjectNameSize);

  bool res = hiveDb.Deatch(name.c_str());
  assert(res);

  fs::remove(GetStorageDir() / name);
}

void DumpAllUpdates(database::SqliteDB &hiveDB, const std::string &name)
{
  auto statement = fmt::format(
      "SELECT * FROM {}.updates;", name);

  database::SqliteStatement dumpCommand(hiveDB, statement.c_str());
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

int main(int argc, char **argv)
{
  //::testing::InitGoogleMock(&argc, argv);
  database::InstallErrorDebugPrint();

  bool checkLeaks = false;
  if(argc > 1 && std::strcmp(argv[1], "--check_for_leaks") == 0)
	checkLeaks = true;
  else
	std::puts(
	    "Run this program with --check_for_leaks to enable "
	    "custom leak checking in the tests.");

#if 0
  if (checkLeaks) {
	auto &listeners = UnitTest::GetInstance()->listeners();
	listeners.Append(new )
  }
#endif

  fs::remove(MAIN_DB_NAME);
  fs::remove_all("test_storage");

  fs::create_directory("test_storage");

  CreateMainDb();
  InsertWorkspace();
  //RemoveWorkspace();

  {
	database::SqliteDB db;
	if(!db.open(MAIN_DB_NAME)) {
	  std::puts("CreateProject() failed!");
	  return 0;
	}

	for(int i = 0; i < 10; i++) {
	  const auto name = fmt::format("project_{}", i);
	  CreateProject(db, name);
	}

	for(int i = 0; i < 10; i++) {
	  const auto name = fmt::format("project_{}", i);
	  DumpAllUpdates(db, name);
	}

	/*for (int i = 0; i < 10; i++) {
	  auto dbName = fmt::format("project_{}", i);
	  DeleteProject(db, dbName);
	}*/
  }

  return 0;
  //return RUN_ALL_TESTS();
}