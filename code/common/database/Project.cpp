// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include <sqlite3.h>

#include "Project.h"

namespace database {

  Project::~Project()
  {
	if(_db)
	  sqlite3_close(_db);
  }

  bool Project::Create(const char *fileName)
  {
	if(sqlite3_open(fileName, &_db) != SQLITE_OK)
	  return false;

	// clang-format off
    static const char query[1024] =
	  R"(CREATE TABLE info (
            infoid INTEGER PRIMARY KEY AUTOINCREMENT, 
            name TEXT NOT NULL, 
            value TEXT NOT NULL
        );
        CREATE TABLE updates (
            updateid INTEGER PRIMARY KEY AUTOINCREMENT, 
            user_guid TEXT NOT NULL,
            created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, 
            message_type BIGINT, 
            data BLOB
        );
        CREATE TABLE users (
            userid INTEGER PRIMARY KEY AUTOINCREMENT, 
            user_guid TEXT NOT NULL, 
            user_name TEXT NOT NULL
        );
        INSERT INTO info (name, value) VALUES('idb_name', ?);
        INSERT INTO info (name, value) VALUES('idb_md5', ?);)";
	// clang-format on

	sqlite3_stmt *stmt = nullptr;
	if(sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
	  sqlite3_close(_db);

	  // TODO: should really also delete the file...
	  return false;
	}

	sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), nullptr);
	sqlite3_bind_text(stmt, 2, md5.c_str(), md5.length(), nullptr);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	return true;
  }

  void Project::AddMessage(const char *sourceId, const protocol::Message *message)
  {
	auto str = fmt::format(
	    "INSERT INTO updates (user_guid, message_type, data) VALUES({}, {}, {});",
	    sourceId, message->msg_type(), message->msg());
	sqlite3_exec(_db, str.c_str(), nullptr, nullptr, nullptr);
  }
} // namespace database