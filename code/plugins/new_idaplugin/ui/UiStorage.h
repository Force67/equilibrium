// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "utils/NetNode.h"

class UiStorage {
public:
  void SetTick(uint32_t newVal);
  int GetTick() const;

  void Save();
  void Load();

private:
  static ssize_t idaapi Event(void *, int, va_list);

  uint32_t _tick;
  noda::NetNode _node;
};
