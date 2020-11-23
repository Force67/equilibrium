// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include <memory>
#include <vector>

#include <network/TCPServer.h>

namespace noda {

  class User {
  public:
	explicit User(uint32_t id, std::string name, std::string guid);

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

  using userptr_t = std::shared_ptr<User>;

  class UserRegistry {
  public:
	userptr_t UserById(network::connectid_t cid);
	userptr_t AddUser(network::connectid_t cid, const std::string &name, const std::string &guid);

	void RemoveUser(network::connectid_t);

	// for for loop
	auto begin() { return _list.begin(); }
	auto end() { return _list.end(); }

	int32_t UserCount() const
	{
	  return static_cast<int32_t>(_list.size());
	}

  private:
	std::vector<userptr_t> _list;
  };
} // namespace noda