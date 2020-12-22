// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sqlite_db.h"
#include "sqlite_statement.h"

int main() {
  database::SqliteDB db;
  bool r = db.open("test.db");

  const auto state = db.ExecuteChecked(
      "create table if not exists workspaces("
      "id integer primary key autoincrement,"
      "name text not null,"
      "desc text not null);"

      "create table if not exists projects("
      "id integer primary key autoincrement,"
      "wks_id integer,"
      "highest_uid integer default 0,"
      "name text not null,"
      "guid text not null);");

  if (state != database::SqliteDB::DBStatus::kSuccess) {

    db.close();
  } else
    db.close();

  return 0;
}