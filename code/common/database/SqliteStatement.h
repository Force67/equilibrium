// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>

struct sqlite3_stmt;

namespace database {

  class SqliteDB;

  class SqliteStatement {
  public:
	explicit SqliteStatement(SqliteDB &, const char *sql);
	~SqliteStatement();

	// must be utf8
	bool Bind(const std::string &);
	bool Bind(int val);
	bool Bind(const void *blob, size_t len);

	std::string ColumnStr(int col);
	const void *ColumnBlob(int col) const;
	int ColumnInt(int col) const;
	int64_t ColumnBigInt(int col) const;

	[[nodiscard]] bool Run();
	[[nodiscard]] inline bool Good() const
	{
	  return _good;
	}

	bool Step();

	template <typename... Args>
	bool BindMany(database::SqliteStatement &cmd)
	{
	  return true;
	}

	template <typename T, typename... Args>
	bool BindMany(database::SqliteStatement &cmd, const T &arg, Args &&... args)
	{
	  if(!cmd.Bind(arg)) {
		return false;
	  }
	  return Bind(cmd, std::forward<Args>(args)...);
	}

  private:
	sqlite3_stmt *_st;
	bool _good;
	int _idx = 1;
  };

} // namespace database