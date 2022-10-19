// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "sqlite_db.h"
#include "test/sqlite_test_suite.h"

namespace database {
namespace {

TEST_F(SqliteTestSuite, ExecuteSql) {
  // invalid statement
  ASSERT_FALSE(db().Execute("create tab foo (a, b"));
  // valid statement
  ASSERT_TRUE(db().Execute("create table foo (a, b)"));
}

TEST_F(SqliteTestSuite, AttachDatabase) {
  auto path = db_path() / "TestDb.db";
  ASSERT_TRUE(!path.empty());

  // TBD
}

}  // namespace
}  // namespace database