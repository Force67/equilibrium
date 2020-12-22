// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>

struct sqlite3;
struct sqlite3_stmt;

namespace database {

class SqliteDB {
  friend class SqliteStatement;
 public:
  // open DB from file
  explicit SqliteDB(const char* fileNameUtf8);
  // construct in unopened state
  SqliteDB();

  ~SqliteDB();

  enum class DBStatus {
	  kSuccess,
	  kUnopened,
	  kIllegalUse,
	  kFailed
  };

  static inline bool CheckStatus(DBStatus s) { return s == DBStatus::kSuccess; }

  bool open(std::string_view fileName);
  bool create(const char* fileNameUtf8);
  void close();

  // unsafe method of executing any SQL
  bool ExecuteUnchecked(const char* sql);

  // execute safe, everything should use this...
  DBStatus ExecuteChecked(const char* sql);

  // attach/detach a child database to main db table
  bool Attach(const std::string& utf8path, const char* alias);
  bool Deatch(const char* alias);

  // current row
  int64_t LastestRowId() const;

  // Error Handler callback
  using errorhandler_t = void(void*, int, const char*);
  static bool InstallErrorHandler(errorhandler_t*);

 private:
  sqlite3* _db;
  bool _good;
};
}  // namespace database