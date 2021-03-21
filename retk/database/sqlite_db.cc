// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <sqlite3.h>
#include <cassert>

#include "sqlite_db.h"
#include "sqlite_statement.h"

namespace database {

static bool IsASCIIWhiteSpace(uint8_t c) {
  // From Encoding Standard:
  // "The ASCII whitespace are code points U+0009, U+000A, U+000C, U+000D, and
  // U+0020."
  return c == 0x09 || c == 0x0A || c == 0x0C || c == 0x0D || c == 0x20;
}

SqliteDB::SqliteDB(const char* fileNameUtf8) {
  _good = sqlite3_open(fileNameUtf8, &_db) == SQLITE_OK;
}

SqliteDB::SqliteDB() : _db(nullptr), _good(false) {}

SqliteDB::~SqliteDB() {
  close();
}

bool SqliteDB::InstallErrorHandler(errorhandler_t* handler) {
  const int rc = sqlite3_config(SQLITE_CONFIG_LOG, handler, nullptr);
  return rc == SQLITE_OK;
}

bool SqliteDB::open(std::string_view view) {
  _good = sqlite3_open_v2(view.data(), &_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                          nullptr) == SQLITE_OK;
  return _good;
}

bool SqliteDB::create(const char* fileNameUtf8) {
  _good = sqlite3_open_v2(fileNameUtf8, &_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                          nullptr) == SQLITE_OK;
  return _good;
}

void SqliteDB::close() {
  if (_db)
    sqlite3_close(_db);
}

bool SqliteDB::ExecuteUnchecked(const char* sql) {
  char* errMsg;
  const int rc = sqlite3_exec(_db, sql, nullptr, nullptr, &errMsg);
  if (rc != 0) {
    int ts = sqlite3_threadsafe();

    __debugbreak();
  }

  return rc == SQLITE_OK;
}

// based off:
// https://github.com/chromium/chromium/blob/dbce9b8c47bd77221cfdd073b6da2bacf2782131/sql/database.cc
SqliteDB::DBStatus SqliteDB::ExecuteChecked(const char* sql) {
  if (!_db)
    return DBStatus::kUnopened;

  int rc = SQLITE_OK;

  while ((rc == SQLITE_OK) && *sql) {
    sqlite3_stmt* stmt;
    const char* leftover;

    rc = sqlite3_prepare_v3(_db, sql, -1, 0, &stmt, &leftover);

    // stop if an error is encountered
    if (rc != SQLITE_OK)
      break;

    sql = leftover;

    if (!stmt)
      continue;

    while ((rc == sqlite3_step(stmt)) == SQLITE_ROW) {
    }

    rc = sqlite3_finalize(stmt);

    while (IsASCIIWhiteSpace(*sql)) {
      sql++;
    }
  }

#if 1
  if (rc != SQLITE_OK)
    __debugbreak();
#endif

  return rc == SQLITE_OK ? DBStatus::kSuccess : DBStatus::kFailed;
}

bool SqliteDB::Attach(const std::string& path, const char* alias) {
  SqliteStatement command(*this, "ATTACH DATABASE ? AS ?");
  if (!command.Good())
    return false;

  // warning: *ensure* that the main DB was opened with the 'SQLITE_OPEN_CREATE'
  // flag, else everything will crash and burn
  // assert(_db->flags & SQLITE_OPEN_CREATE);

  bool rs;
  rs = command.Bind(path);
  rs = command.Bind(alias);
  if (!rs)
    return false;

  return command.Run();
}

bool SqliteDB::Deatch(const char* alias) {
  SqliteStatement commend(*this, "DETACH DATABASE ?");
  if (!commend.Good())
    return false;

  if (!commend.Bind(alias))
    return false;

  return commend.Run();
}

int64_t SqliteDB::LastestRowId() const {
  return sqlite3_last_insert_rowid(_db);
}
}  // namespace database