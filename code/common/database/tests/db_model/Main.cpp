// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <algorithm>
#include <filesystem>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "database/SqliteDB.h"
#include "database/SqliteStatement.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define MAIN_DB_NAME "hive"

namespace fs = std::filesystem;

void RunAllTests(const fs::path &, database::SqliteDB &db);

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

int main(int argc, char **argv)
{
  testing::InitGoogleMock(&argc, argv);

  //::testing::InitGoogleMock(&argc, argv);
  database::InstallErrorDebugPrint();

  bool checkLeaks = false;
  if(argc > 1 && std::strcmp(argv[1], "--check_for_leaks") == 0)
	checkLeaks = true;
  else
	std::puts(
	    "Run this program with --check_for_leaks to enable "
	    "custom leak checking in the tests.");

  auto path = GetStorageDir();
  if(fs::exists(path)) {
	fs::remove_all(path);
  }

  fs::create_directory(path);

  auto dbPath = (path / MAIN_DB_NAME).u8string();
  database::SqliteDB db;
  assert(db.open(dbPath.c_str()));

  RunAllTests(path, db);

  return 0;
  //return RUN_ALL_TESTS();
}