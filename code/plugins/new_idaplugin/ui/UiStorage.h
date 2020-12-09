// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "utils/NetNode.h"

class UiStorage {
public:
  UiStorage();
  ~UiStorage();

private:
  static ssize_t idaapi Event(void *, int, va_list);

  noda::NetNode _node;
};
