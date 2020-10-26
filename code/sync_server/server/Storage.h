// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <filesystem>
#include "database/SqliteDB.h"

namespace noda {

  namespace fs = std::filesystem;

  struct StorageConfig {
  };

  class NdUser;

  class Storage {
  public:
	// Admin Perms are always assigned to
	// the user who created the workspace/project
	// and can be given to other users by this user
	enum class Perms : int {
	  Create = 1 << 0,
	  Delete = 1 << 1,

	  Admin = Create | Delete,
	};

	struct ProjectInfo {
	  std::string name;
	  std::string md5;

	};

	Storage();

	// creates the db index hive
	bool Initialize(const StorageConfig&);

	bool AddWorkspace(const std::string &);
	bool RemoveWorkspace(const std::string &, bool withProjects);

	bool MakeProject(const std::string &, const std::string &, bool &created);

	bool AddUser(const std::string &, const NdUser&);

	static const fs::path &GetStorageDir() noexcept;

  private:
	database::SqliteDB _db;
  };

} // namespace noda