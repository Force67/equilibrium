// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "sqlite_db.h"
#include "sqlite_statement.h"
#include "test/sqlite_test_suite.h"

namespace database {
namespace {

TEST_F(SqliteTestSuite, AssignStatement) {
  EXPECT_TRUE(
      db().Execute("create table if not exists stmt(a, b)"));

  SqliteStatement s1(db(), "insert into stmt_fail (a, b) value(?, ?)");
  EXPECT_FALSE(s1.Good());

  SqliteStatement s2(db(), "insert into stmt (a, b) value(?, ?)");
  EXPECT_FALSE(s2.Good());
}

TEST_F(SqliteTestSuite, BindStatement) {
  EXPECT_TRUE(db().Execute("create table if not exists stmt(a, b)"));

  // TBD

  #if 0
  SqliteStatement s1(db(), "insert into stmt (a, b) value(?, ?)");
  ASSERT_TRUE(s1.Good());

  s1.Bind();
  #endif
}

TEST_F(SqliteTestSuite, RunStatement) {
  EXPECT_TRUE(db().Execute("create table if not exists stmt(a, b)"));


}

}  // namespace
}  // namespace database