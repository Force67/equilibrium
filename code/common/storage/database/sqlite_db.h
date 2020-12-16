// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

struct sqlite3;
struct sqlite3_stmt;

#include <fmt/format.h>
#include <string>

#include <filesystem>

namespace database {

class SqliteDB;

enum class SqliteStatus {
  Success,
  IllegalUse,
  Failed,
};

inline bool CheckStatus(SqliteStatus s) {
  return s == SqliteStatus::Success;
}

class SqliteDB {
  friend class SqliteStatement;

 public:
  // from file
  explicit SqliteDB(const char* fileNameUtf8);

  // unopened
  SqliteDB();

  ~SqliteDB();

  bool open(std::string_view fileName);
  bool create(const char* fileNameUtf8);

  void close();

  bool Execute(const char* sql);

  SqliteStatus ExecuteAndNotifyError(const char* statement);

  bool Attach(const std::string& utf8path, const char* alias);
  bool Deatch(const char* alias);

  int64_t LastestRowId() const;

 private:
  sqlite3* _db;
  bool _good;
};

void InstallErrorDebugPrint();
}  // namespace database