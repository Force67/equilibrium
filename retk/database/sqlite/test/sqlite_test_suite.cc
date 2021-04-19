// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <ctime>
#include <fmt/format.h>
#include "sqlite_test_suite.h"

namespace database {

void SqliteTestSuite::SetUp() {
  ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
  ASSERT_TRUE(db_.open(db_path().u8string()));
}

void SqliteTestSuite::TearDown() {
  db_.close();
}

base::fs::path SqliteTestSuite::db_path() {
  return temp_dir_.Path() / "SqliteTestSuite.db";
}

}  // namespace database