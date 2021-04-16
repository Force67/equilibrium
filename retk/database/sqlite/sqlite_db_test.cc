// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sqlite_db.h"
#include "test/sqlite_test_suite.h"

namespace database {
namespace {

TEST_F(SqliteTestSuite, ExecuteChecked) {
  // invalid statement
  ASSERT_FALSE(db().ExecuteChecked("create tab foo (a, b"));
  // valid statement
  ASSERT_TRUE(db().ExecuteChecked("create table foo (a, b)"));
}

TEST_F(SqliteTestSuite, ExecuteUnchecked) {
  // invalid statement
  ASSERT_FALSE(db().ExecuteUnchecked("create tab bar (a, b"));
  // valid statement
  ASSERT_TRUE(db().ExecuteUnchecked("create table bar (a, b)"));
}

}  // namespace
}  // namespace database