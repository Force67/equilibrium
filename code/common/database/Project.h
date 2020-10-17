// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include "../moc_protocol/Message_generated.h"

struct sqlite3;

namespace database {

  struct Project {
	~Project();

	std::string md5;
	std::string name;

	// those two keep the project loaded!
	bool Create(const char *name);

	bool Open(const char *name);

	void AddMessage(const char *sourceId,
	                const protocol::Message *);

  private:
	sqlite3 *_db = nullptr;
  };
} // namespace database