// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <sqlite3.h>

#include "SqliteDB.h"
#include "SqliteStatement.h"

namespace database {

  SqliteStatement::SqliteStatement(SqliteDB &db, const char *sql)
  {
	const int rc = sqlite3_prepare_v3(db.GetPrivateObject(), sql, -1, 0, &_st, nullptr);
	_good = rc == SQLITE_OK;
  }

  SqliteStatement::~SqliteStatement()
  {
	if(_st) {
	  sqlite3_clear_bindings(_st);
	  sqlite3_reset(_st);
	  sqlite3_finalize(_st);
	}
  }

  bool SqliteStatement::Run()
  {
	const int rc = sqlite3_step(_st);
	return rc == SQLITE_DONE;
  }

  bool SqliteStatement::Step()
  {
	const int rc = sqlite3_step(_st);
	return rc == SQLITE_ROW;
  }

  bool SqliteStatement::Bind(const std::string &str)
  {
	const int rc = sqlite3_bind_text(_st, _idx, str.c_str(),
	                                 static_cast<int>(str.length()),
	                                 SQLITE_TRANSIENT);

	_idx++;
	return rc == SQLITE_OK;
  }

  bool SqliteStatement::Bind(int val)
  {
	const int rc = sqlite3_bind_int(_st, _idx, val);
	_idx++;

	return rc == SQLITE_OK;
  }

  bool SqliteStatement::Bind(const void *blob, size_t len)
  {
	const int rc = sqlite3_bind_blob(_st, _idx, blob, static_cast<int>(len), SQLITE_TRANSIENT);
	_idx++;

	return rc == SQLITE_OK;
  }

  std::string SqliteStatement::ColumnStr(int col)
  {
	const char *str = reinterpret_cast<const char *>(
	    sqlite3_column_text(_st, col));
	int len = sqlite3_column_bytes(_st, col);
  
	std::string result;
	if(str && len > 0)
	  result.assign(str, len);


	return result;
  }

  const void *SqliteStatement::ColumnBlob(int col) const
  {
	return sqlite3_column_blob(_st, col);
  }

  int SqliteStatement::ColumnInt(int col) const 
  {
	return sqlite3_column_int(_st, col);
  }

  int64_t SqliteStatement::ColumnBigInt(int col) const 
  {
	return sqlite3_column_int64(_st, col);
  }
} // namespace database