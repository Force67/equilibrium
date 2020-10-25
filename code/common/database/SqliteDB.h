// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

struct sqlite3;
struct sqlite3_stmt;

#include <string>
#include <fmt/format.h>

#include <filesystem>

namespace database {

  class SqliteDB;

  enum class SqliteStatus {
	Success,
	IllegalUse,
	Failed,
  };

  inline bool CheckStatus(SqliteStatus s)
  {
	return s == SqliteStatus::Success;
  }

  class SqliteDB {
  public:
	// from file
	explicit SqliteDB(const char *fileNameUtf8);

	// unopened
	SqliteDB();

	~SqliteDB();

	bool open(const char *fileNameUtf8);
	bool create(const char *fileNameUtf8);

	void close();

	bool ExecuteOnly(const char *text);

	/*template <typename... Args>
	void Execute(std::string_view view, Args... args)
	{
	  auto str = fmt::format(view, args);
	  ExecuteOnly(str.c_str());
	}*/

	SqliteStatus ExecuteAndNotifyError(const char *statement);

	bool Attach(const std::string &utf8path, const char *alias);
	bool Deatch(const char *alias);

	inline sqlite3 *GetPrivateObject()
	{
	  return _db;
	}

  private:
	sqlite3 *_db;
	bool _good;
  };

  void InstallErrorDebugPrint();
} // namespace database