// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

// Purpose: This implements the primary data storage for NODA update data.

#include "SqliteStatement.h"
#include "StorageModel.h"

namespace database {

  bool RootDB::Initialize(const std::string &path)
  {
	if(!_db.open(path)) {
	  return false;
	}

	bool res = _db.Execute(
	    "create table if not exists workspaces("
	    "id integer primary key autoincrement,"
	    "name text not null,"
	    "desc text not null);"

	    "create table if not exists projects("
	    "id integer primary key autoincrement,"
	    "wks_id integer,"
	    "highest_uid integer default 0,"
	    "name text not null,"
	    "guid text not null);");

	if(!res) {
	  _db.close();
	  return false;
	}

	return true;
  }

  bool RootDB::CreateWorkspace(const std::string &name, const std::string &desc)
  {
	database::SqliteStatement command(_db, "insert into workspaces (name, desc) values(?,?)");
	if(!command.Good())
	  return false;

	bool res;
	res = command.Bind(name);
	res = command.Bind(desc);
	return res && command.Run();
  }

  bool RootDB::CreateProject(int32_t wksId, const std::string &name, const std::string &guid)
  {
	database::SqliteStatement command(_db, "insert into projects (wks_id, name, guid) values(?,?,?)");
	if(!command.Good())
	  return false;

	bool res;
	res = command.Bind(wksId);
	res = command.Bind(name);
	res = command.Bind(guid);

	return res && command.Run();
  }

  bool RootDB::UpdateHUID(int32_t newest)
  {
	/*database::SqliteStatement command(_db, "insert into projects (wks_id, name, guid) values(?,?,?)");
	if(!command.Good())
	  return false;*/

	return false;
  }

  bool NodaDB::Initialize(const std::string &path)
  {
	if(!_db.open(path)) {
	  return false;
	}

	bool res = _db.Execute(
	    "create table if not exists users("
	    "id integer primary key autoincrement,"
	    "name text not null,"
	    "guid text not null,"
	    "role text not null);"

	    "create table if not exists updates("
	    "id integer primary key autoincrement,"
	    "type integer,"
	    "user_id integer,"
	    "timestamp datetime default current_timestamp,"
	    "data blob);"

	    "create table if not exists files("
	    "id integer primary key autoincrement,"
	    "user_id integer,"
	    "type integer,"
	    "size integer,"
	    "name text not null);");

	if(!res) {
	  _db.close();
	  return false;
	}

	return true;
  }

  bool NodaDB::AddUser(const std::string &name, const std::string &guid)
  {
	database::SqliteStatement command(_db, "insert into users (name, guid) VALUES(?,?)");
	if(!command.Good())
	  return false;

	//bool res = command.BindMany(command, name, guid);
	bool res;
	res = command.Bind(name);
	res = command.Bind(guid);

	return res && command.Run();
  }

  bool NodaDB::AddUpdate(int32_t type, int32_t user_id, const void *data, size_t dataLen)
  {
	database::SqliteStatement command(_db, "insert into updates (type, user_id, data) VALUES(?,?,?)");
	if(!command.Good())
	  return false;

	bool res;
	res = command.Bind(type);
	res = command.Bind(user_id);
	res = command.Bind(data, dataLen);

	return res && command.Run();
  }

} // namespace database