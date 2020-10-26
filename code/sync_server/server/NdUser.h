// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include <memory>

namespace noda {

  class NdUser {
  public:
	explicit NdUser(uint32_t id, std::string name, std::string guid);

	uint32_t Id() const
	{
	  return _netId;
	}

	auto &Name() const
	{
	  return _name;
	}

	auto &Guid() const
	{
	  return _guid;
	}

  private:
	uint32_t _netId;

	std::string _guid;
	std::string _name;
  };

  using userptr_t = std::shared_ptr<NdUser>;
} // namespace noda