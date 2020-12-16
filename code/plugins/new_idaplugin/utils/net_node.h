// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Pch.h"

namespace noda {

// thin wrapper around netnode to make it easier accessible
class NetNode {
 public:
  NetNode(const char* name) { _node = netnode(name, -1, true); }

  NetNode(const char* name, bool docreate) {
    _node = netnode(name, -1, docreate);
  }

  NetNode() { _node = BADNODE; }

  NetNode(const NetNode& node) { _node = node._node; }

  inline bool open() const { return nodeidx_t() != BADNODE; }

  void clear() { _node = BADNODE; }

  // check value bool operator

  bool Create(const char* name) { return _node.create(name, -1); }

  template <typename T>
  T LoadScalar(nodeidx_t idx) {
    T temp;
    _node.supval(idx, &temp, sizeof(T));
    return temp;
  }

  template <typename T>
  T LoadScalar(nodeidx_t idx, T fallback) {
    T temp = fallback;
    _node.supval(idx, &temp, sizeof(T));
    return temp;
  }

  template <typename T>
  bool StoreScalar(nodeidx_t idx, T t) {
    return _node.supset(idx, reinterpret_cast<const void*>(&t), sizeof(T));
  }

 private:
  netnode _node;
};

}  // namespace noda