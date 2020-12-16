#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "utils/NetNode.h"

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