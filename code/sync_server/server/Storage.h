// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <filesystem>
#include "database/SqliteDB.h"

namespace noda {

  namespace fs = std::filesystem;

  struct StorageConfig {
  };

  class Storage {
  public:
	Storage();

	bool Initialize();

	void AddWorkspace(const std::string &);
	void RemoveWorkspace(const std::string &);

	static const fs::path &GetStorageDir() noexcept;

  private:
	database::SqliteDB _db;
  };

} // namespace noda