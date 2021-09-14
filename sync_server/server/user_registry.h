// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <network/base/peer_base.h>

namespace sync_server {

class User {
 public:
  explicit User(network::PeerId, std::string name, std::string guid);

  network::PeerId Id() const { return _cid; }

  auto& Name() const { return _name; }
  auto& Guid() const { return _guid; }

 private:
  network::PeerId _cid;

  std::string _guid;
  std::string _name;
};

using userptr_t = std::shared_ptr<User>;

class UserRegistry {
 public:
  userptr_t UserById(network::PeerId cid);
  userptr_t AddUser(network::PeerId cid,
                    const std::string& name,
                    const std::string& guid);

  void RemoveUser(network::PeerId);

  // for for loop
  auto begin() { return _list.begin(); }
  auto end() { return _list.end(); }

  int32_t UserCount() const { return static_cast<int32_t>(_list.size()); }

 private:
  std::vector<userptr_t> _list;
};
}  // namespace sync_server