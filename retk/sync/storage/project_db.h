// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "database/sqlite_db.h"

namespace sync::storage {

class MainDb;

class ProjectDb {
 public:
  explicit ProjectDb(MainDb&);

  bool Initialize(const std::string& path);

  bool AddUser(const std::string& name, const std::string& guid);
  bool AddUpdate(int32_t type,
                 int32_t user_id,
                 const void* data,
                 size_t dataLen);

 private:
  database::SqliteDB db_;
  MainDb &parent_;
};
}