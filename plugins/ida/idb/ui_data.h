// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "idb_store.h"

class UiData final : public IdbNode {
 public:
  UiData();

  void Save() override;
  void Load() override;

  static bool SeenBefore();

 private:
  enum NodeIndex : nodeidx_t { StorageVersion, Tick, IdbUsageFlags, RunIndex };

 public:
  // last timestamp
  uint32_t tick = 0;
  // last used run action
  int last_run_index = 0;
};