// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "project_db.h"
#include "database/sqlite_statement.h"

namespace sync::storage {
using namespace database;

ProjectDb::ProjectDb(MainDb& parent) : parent_(parent) {
}

bool ProjectDb::Initialize(const std::string& path) {
  if (!db_.open(path)) {
    return false;
  }

  const auto status = db_.ExecuteChecked(
      "create table if not exists users("
      "id integer primary key autoincrement,"
      "name text not null,"
      "guid text not null,"
      "role text not null);"

      "create table if not exists updates("
      "id integer primary key autoincrement,"
      "type integer,"
      "user_id integer,"
      "timestamp datetime default current_timestamp,"
      "data blob);"

      "create table if not exists files("
      "id integer primary key autoincrement,"
      "user_id integer,"
      "type integer,"
      "size integer,"
      "name text not null);");

  if (status != SqliteDB::DBStatus::kSuccess) {
    db_.close();
    return false;
  }

  return true;
}

bool ProjectDb::AddUser(const std::string& name, const std::string& guid) {
  database::SqliteStatement command(
      db_, "insert into users (name, guid) VALUES(?,?)");
  if (!command.Good())
    return false;

  // bool res = command.BindMany(command, name, guid);
  bool res;
  res = command.Bind(name);
  res = command.Bind(guid);

  return res && command.Run();
}

bool ProjectDb::AddUpdate(int32_t type,
                       int32_t user_id,
                       const void* data,
                       size_t dataLen) {
  database::SqliteStatement command(
      db_, "insert into updates (type, user_id, data) VALUES(?,?,?)");
  if (!command.Good())
    return false;

  bool res;
  res = command.Bind(type);
  res = command.Bind(user_id);
  res = command.Bind(data, dataLen);

  return res && command.Run();
}
}