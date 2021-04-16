// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <ctime>
#include <fmt/format.h>
#include "sqlite_test_suite.h"

namespace database {

void SqliteTestSuite::SetUp() {
  // a rather bad way of solving this, until we have a base temp dir
  // TODO: base temp dir.

  auto name = fmt::format("SqliteTestSuite_{}.db", time(nullptr));
  ASSERT_TRUE(db_.open(name));
}

void SqliteTestSuite::TearDown() {
  db_.close();
}
}  // namespace database