// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "idb_store.h"

class WorkbenchData final : public IdbNode {
 public:
  WorkbenchData();

 private:
  void Save() override;
  void Load() override;

  enum NodeIndex : nodeidx_t { StorageVersion, Name };

 public:
  qstring sym_table_path;
};