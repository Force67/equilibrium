// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "database/sqlite_db.h"

namespace sync::storage {

class MainDb {
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
}