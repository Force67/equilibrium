// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/strings/string_ref.h>

struct sqlite3_stmt;

namespace database {

class SqliteDb;

class SqliteStatement {
 public:
  explicit SqliteStatement(SqliteDb&, const char* sql);
  ~SqliteStatement();

  // must be utf8
  bool Bind(base::StringRef);
  bool Bind(int val);
  bool Bind(const void* blob, size_t len);

  base::String ColumnStr(i32 col);
  const void* ColumnBlob(i32 col) const;
  int ColumnInt(i32 col) const;
  int64_t ColumnBigInt(i32 col) const;

  [[nodiscard]] bool Run();
  [[nodiscard]] inline bool Good() const { return good_; }

  bool Step();

  template <typename... Args>
  bool BindMany(database::SqliteStatement& cmd) {
    return true;
  }

  template <typename T, typename... Args>
  bool BindMany(database::SqliteStatement& cmd, const T& arg, Args&&... args) {
    if (!cmd.Bind(arg)) {
      return false;
    }
    return Bind(cmd, std::forward<Args>(args)...);
  }

 private:
  sqlite3_stmt* st_;
  bool good_;
  i32 idx_ = 1;
};

}  // namespace database