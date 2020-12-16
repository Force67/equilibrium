// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "database/sqlite_db.h"

namespace database {

class RootDB {
 public:
  bool Initialize(const std::string& path);

  bool CreateWorkspace(const std::string& name, const std::string& desc);
  bool CreateProject(int32_t wksId,
                     const std::string& name,
                     const std::string& guid);

  bool UpdateHUID(int32_t newest);

 private:
  database::SqliteDB _db;
};

class NodaDB {
 public:
  bool Initialize(const std::string& path);

  bool AddUser(const std::string& name, const std::string& guid);
  bool AddUpdate(int32_t type,
                 int32_t user_id,
                 const void* data,
                 size_t dataLen);

 private:
  database::SqliteDB _db;
};
}  // namespace database