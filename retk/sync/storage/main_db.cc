// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "main_db.h"
#include "database/sqlite_statement.h"

namespace sync::storage {
using namespace database;

bool MainDb::Initialize(const std::string& path) {
  if (!_db.open(path)) {
    return false;
  }

  const auto status = _db.ExecuteChecked(
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

  if (status != SqliteDB::DBStatus::kSuccess) {
    _db.close();
    return false;
  }

  return true;
}

bool MainDb::CreateWorkspace(const std::string& name, const std::string& desc) {
  database::SqliteStatement command(
      _db, "insert into workspaces (name, desc) values(?,?)");
  if (!command.Good())
    return false;

  bool res;
  res = command.Bind(name);
  res = command.Bind(desc);
  return res && command.Run();
}

bool MainDb::CreateProject(int32_t wksId,
                           const std::string& name,
                           const std::string& guid) {
  database::SqliteStatement command(
      _db, "insert into projects (wks_id, name, guid) values(?,?,?)");
  if (!command.Good())
    return false;

  bool res;
  res = command.Bind(wksId);
  res = command.Bind(name);
  res = command.Bind(guid);

  return res && command.Run();
}

bool MainDb::UpdateHUID(int32_t newest) {
  /*database::SqliteStatement command(_db, "insert into projects (wks_id, name,
  guid) values(?,?,?)"); if(!command.Good()) return false;*/

  return false;
}
}