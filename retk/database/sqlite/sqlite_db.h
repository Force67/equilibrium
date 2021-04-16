// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>

// forward declared, since we don't want to poison
// our global namespace with our particular sqlite
// implementation.
struct sqlite3;
struct sqlite3_stmt;

namespace database {

class SqliteDb {
 public:
  // open DB from file
  explicit SqliteDb(const char* fileNameUtf8);
  // construct in unopened state
  SqliteDb();

  ~SqliteDb();

  // TODO: replace this string view with
  // c_str view.
  bool open(std::string_view fileName);
  void close();

  // unsafe method of executing any SQL
  bool ExecuteUnchecked(const char* sql);

  // execute safe, everything should use this...
  bool ExecuteChecked(const char* sql);

  // attach/detach a child database to main db table
  bool Attach(const std::string& utf8path, const char* alias);
  bool Deatch(const char* alias);

  // current row
  int64_t LastestRowId() const;

  static bool SetGlobalConfig();

  sqlite3* handle() { return handle_; }

 private:
  sqlite3* handle_;
  bool good_;
};
}  // namespace database