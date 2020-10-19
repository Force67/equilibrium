// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>

namespace noda {

  struct Client {
	std::string name;
	std::string guid;
	uint32_t id;
  };

  using clientPtr = std::shared_ptr<Client>;
} // namespace noda