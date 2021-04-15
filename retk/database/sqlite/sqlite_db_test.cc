// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <sqlite3.h>
#include "sqlite_db.h"
#include "sqlite_statement.h"

#include <gtest/gtest.h>

namespace database {

namespace {

TEST(SqliteDatabaseTest, OpenDatabase) {
  database::SqliteDB db;
  EXPECT_TRUE(db.open("test.db"));
}

TEST(SqliteDatabaseTest, ExecuteChecked) {
  // TODO: move to global database object, as seen on:
  // https://github.com/chromium/chromium/blob/d7da0240cae77824d1eda25745c4022757499131/sql/database_unittest.cc
  database::SqliteDB db;
  EXPECT_TRUE(db.open("test.db"));

  constexpr char kSql[] =
      "create table if not exists workspaces("
      "id integer primary key autoincrement,"
      "name text not null,"
      "desc text not null);"

      "create table if not exists projects("
      "id integer primary key autoincrement,"
      "wks_id integer,"
      "highest_uid integer default 0,"
      "name text not null,"
      "guid text not null);";

  EXPECT_EQ(db.ExecuteChecked(kSql), SqliteDB::DBStatus::kSuccess);
  // TODO: only close on error!
  db.close();
}

TEST(SqliteDatabaseTest, ExecuteUnChecked) {
  // TODO: move to global database object, as seen on:
  // https://github.com/chromium/chromium/blob/d7da0240cae77824d1eda25745c4022757499131/sql/database_unittest.cc
  database::SqliteDB db;
  EXPECT_TRUE(db.open("test2.db"));

  constexpr char kSql[] =
      "create table if not exists workspaces("
      "id integer primary key autoincrement,"
      "name text not null,"
      "desc text not null);"

      "create table if not exists projects("
      "id integer primary key autoincrement,"
      "wks_id integer,"
      "highest_uid integer default 0,"
      "name text not null,"
      "guid text not null);";

  EXPECT_TRUE(db.ExecuteUnchecked(kSql));
  // TODO: only close on error!
  db.close();
}

}  // namespace

}  // namespace database