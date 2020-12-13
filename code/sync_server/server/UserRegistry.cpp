// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UserRegistry.h"

namespace noda {

  User::User(network::connectid_t id, std::string name, std::string guid) :
      _cid(id)
  {
	_name = std::move(name);
	_guid = std::move(guid);
  }

  userptr_t UserRegistry::UserById(network::connectid_t cid)
  {
	auto it = std::find_if(_list.begin(), _list.end(), [&](userptr_t &user) {
	  return user->Id() == cid;
	});

	if(it == _list.end())
	  return nullptr;

	return *it;
  }

  userptr_t UserRegistry::AddUser(network::connectid_t cid, const std::string &name, const std::string &guid)
  {
	userptr_t user = std::make_shared<User>(cid, name, guid);

	// TODO: db lookup in context...
	return _list.emplace_back(user);
  }

  void UserRegistry::RemoveUser(network::connectid_t cid)
  {
	auto it = std::find_if(_list.begin(), _list.end(), [&](userptr_t &user) {
	  return user->Id() == cid;
	});

	if(it != _list.end())
	  _list.erase(it);
  }
} // namespace noda