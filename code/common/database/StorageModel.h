// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "SqliteDB.h"

namespace database {

  class StorageModel {
  public:
	StorageModel();

	bool Initialize(const std::string &path);

  private:
	database::SqliteDB _db;
  };
} // namespace database