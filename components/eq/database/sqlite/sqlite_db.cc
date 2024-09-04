// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <sqlite3.h>
#include <base/logging.h>

#include <database/sqlite/sqlite_db.h>
#include <database/sqlite/sqlite_statement.h>

namespace database {

static bool IsASCIIWhiteSpace(uint8_t c) {
  // From Encoding Standard:
  // "The ASCII whitespace are code points U+0009, U+000A, U+000C, U+000D, and
  // U+0020."
  return c == 0x09 || c == 0x0A || c == 0x0C || c == 0x0D || c == 0x20;
}

bool SqliteDb::SetGlobalConfig() {
  static auto errorHandler = [](void*, int line, const char* msg) {
    BASE_LOG_ERROR("SqliteError <line {}> {}", line, msg);
  };

  return sqlite3_config(SQLITE_CONFIG_LOG, errorHandler, nullptr) == SQLITE_OK;
}

SqliteDb::SqliteDb(const char* fileNameUtf8) {
  Open(fileNameUtf8);
}

SqliteDb::SqliteDb() : handle_(nullptr), good_(false) {}

SqliteDb::~SqliteDb() {
  Close();
}

bool SqliteDb::Open(base::StringRef path) {
  // .c_str() will assert when the nterm is missing!

  int rc = sqlite3_open_v2(path.c_str(), &handle_,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

  if (rc != SQLITE_OK) {
    // cannot expand until handle is assigned.
    rc = sqlite3_extended_errcode(handle_);
    BASE_LOG_ERROR("Failed to open db: {}", rc);

    // TODO: detail
  }

  return (good_ = rc == SQLITE_OK);
}

void SqliteDb::Close() {
  if (handle_)
    sqlite3_close(handle_);
}

bool SqliteDb::Execute(const char* sql) {
  if (!handle_) {
    BASE_LOG_ERROR("Execute: Invalid database handle");
    return false;
  }

  int rc = SQLITE_OK;
  while ((rc == SQLITE_OK) && *sql) {
    sqlite3_stmt* stmt;
    const char* leftover;

    rc = sqlite3_prepare_v3(handle_, sql, -1, 0, &stmt, &leftover);

    // stop if an error is encountered
    if (rc != SQLITE_OK)
      break;

    sql = leftover;

    if (!stmt)
      continue;

    // skip over any rows
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    };

    rc = sqlite3_finalize(stmt);
    while (IsASCIIWhiteSpace(*sql)) {
      sql++;
    }
  }

  return rc == SQLITE_OK;
}

bool SqliteDb::Attach(const base::String& path, const char* alias) {
  SqliteStatement command(*this, "attach database ? as ?");
  if (!command.Good())
    return false;

  // warning: *ensure* that the main DB was opened with the 'SQLITE_OPEN_CREATE'
  // flag, else everything will crash and burn
  // BUGCHECK(handle_->flags & SQLITE_OPEN_CREATE);

  bool rs;
  rs = command.Bind(path);
  rs = command.Bind(alias);
  if (!rs)
    return false;

  return command.Run();
}

bool SqliteDb::Deatch(const char* alias) {
  SqliteStatement commend(*this, "detach database ?");
  if (!commend.Good())
    return false;

  if (!commend.Bind(alias))
    return false;

  return commend.Run();
}

int64_t SqliteDb::LastestRowId() const {
  return sqlite3_last_insert_rowid(handle_);
}
}  // namespace database