// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <sqlite3.h>

#include "sqlite_db.h"
#include "sqlite_statement.h"

namespace database {

SqliteStatement::SqliteStatement(SqliteDb& db, const char* sql) {
  const int rc = sqlite3_prepare_v3(db.handle(), sql, -1, 0, &st_, nullptr);
  good_ = rc == SQLITE_OK;
}

SqliteStatement::~SqliteStatement() {
  if (st_) {
    sqlite3_clear_bindings(st_);
    sqlite3_reset(st_);
    sqlite3_finalize(st_);
  }
}

bool SqliteStatement::Run() {
  const int rc = sqlite3_step(st_);

  // const char* code = sqlite3_errmsg(g_debug->db_);
  return rc == SQLITE_DONE;
}

bool SqliteStatement::Step() {
  const int rc = sqlite3_step(st_);
  return rc == SQLITE_ROW;
}

bool SqliteStatement::Bind(base::StringRef str) {
  const int rc = sqlite3_bind_text(
      st_, idx_, str.c_str(), static_cast<int>(str.length()), SQLITE_TRANSIENT);

  idx_++;
  return rc == SQLITE_OK;
}

bool SqliteStatement::Bind(int val) {
  const int rc = sqlite3_bind_int(st_, idx_, val);
  idx_++;

  return rc == SQLITE_OK;
}

bool SqliteStatement::Bind(const void* blob, size_t len) {
  const int rc = sqlite3_bind_blob(st_, idx_, blob, static_cast<int>(len),
                                   SQLITE_TRANSIENT);
  idx_++;

  return rc == SQLITE_OK;
}

base::String SqliteStatement::ColumnStr(i32 col) {
  const char* str =
      reinterpret_cast<const char*>(sqlite3_column_text(st_, col));
  int len = sqlite3_column_bytes(st_, col);

  base::String result;
  if (str && len > 0)
    result.assign(str, len);

  return result;
}

const void* SqliteStatement::ColumnBlob(i32 col) const {
  return sqlite3_column_blob(st_, col);
}

int SqliteStatement::ColumnInt(i32 col) const {
  return sqlite3_column_int(st_, col);
}

int64_t SqliteStatement::ColumnBigInt(i32 col) const {
  return sqlite3_column_int64(st_, col);
}
}  // namespace database