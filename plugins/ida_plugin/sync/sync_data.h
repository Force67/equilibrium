#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "utils/net_node.h"

class SessionStore {
 public:
  SessionStore();
  ~SessionStore();

  int LocalVersion() const;

  void BumpVersion();

 private:
  static ssize_t idaapi Event(void*, int, va_list);

  void Load();
  void Save();

  int _localVersion = 0;

  noda::NetNode _node;
};