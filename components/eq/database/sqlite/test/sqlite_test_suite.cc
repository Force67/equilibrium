// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <ctime>
#include <fmt/format.h>
#include "sqlite_test_suite.h"

namespace database {

void SqliteTestSuite::SetUp() {
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());

  //const std::string path = db_path().c_str();
  __debugbreak();
  std::string path = "";
  ASSERT_TRUE(db_.Open(path));
}

void SqliteTestSuite::TearDown() {
  db_.Close();
}

base::Path SqliteTestSuite::db_path() {
  return temp_dir_.path() / "SqliteTestSuite.db";
}

}  // namespace database