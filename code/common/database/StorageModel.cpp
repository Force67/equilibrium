// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// Purpose: This implements the primary data storage for NODA update data.

#include "StorageModel.h"

namespace database {

  StorageModel::StorageModel()
  {
  }

  bool StorageModel::Initialize(const std::string &path)
  {
	if(!_db.open(path)) {
	  return false;
	}

	bool res = _db.Execute(
	    "create table if not exists buckets("
	    "id integer primary key autoincrement,"
	    "name text not null);"

	    "create table if not exists file_root("
	    "id integer primary key autoincrement,"
	    "prj_id integer,"
	    "type integer," // reserved for future use
	    "name text not null,"
	    "path text not null);"); // initial hive layout

	if(!res) {
	  _db.close();
	  return false;
	}

	return true;
  }


} // namespace database