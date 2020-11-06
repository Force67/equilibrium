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
  class Storage;

  class NodaDb {
  public:
	NodaDb(Storage &, std::string &name);

	// Admin Perms are always assigned to
	// the user who created the workspace/project
	// and can be given to other users by this user
	enum class Perms : int {
	  Create = 1 << 0,
	  Delete = 1 << 1,

	  None = 0,
	  Admin = Create | Delete,
	};

	void AddRef()
	{
	  _rc++;
	}

	void DeRef()
	{
	  _rc--;
	}

	auto &Name() const
	{
	  return _name;
	}

	bool Open();
	bool AddUser(NdUser &, Perms);

	//void AddUpdate();
  private:
	// doesnt need to be atomic *currently*
	int _rc = 0;

	Storage &_parent;
	std::string _name;
  };

  class Storage {
	friend class NodaDb;

  public:
	enum FileType : int {
	  NodaDatabase,
	  IdaDatabase,
	};

	struct ProjectInfo {
	  std::string name;
	  std::string md5;
	};

	Storage();

	// creates the db index hive
	bool Initialize(const StorageConfig &);

	// project index
	bool AddBucket(const std::string &);
	bool RemBucket(const std::string &);

	int BucketByName(const std::string &name);

  private:
	database::SqliteDB _db;
  };

} // namespace noda