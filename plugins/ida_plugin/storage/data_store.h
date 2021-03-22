#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <vector>
#include "utils/net_node.h"

class NodeTraits {
 public:
  NodeTraits();
  ~NodeTraits();

  virtual const char* 
	  const GetName() = 0;

  virtual void Save() = 0;
  virtual void Load() = 0;

 protected:
  noda::NetNode _node{};
};

class DataStore {
 public:
  DataStore();
  ~DataStore();

  void RegisterNode(NodeTraits*);
  void DumpNodes();

 private:
  static ssize_t idaapi Event(void*, int, va_list);

 private:
  std::vector<NodeTraits*> nodes_;
};