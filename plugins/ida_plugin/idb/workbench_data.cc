// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "workbench_data.h"

namespace {
constexpr char kStorageNodeId[] = "$ retk_wkb_data";
constexpr int kUiStorageVersion = 1;
}  // namespace

WorkbenchData::WorkbenchData() : IdbNode(kStorageNodeId) {}

void WorkbenchData::Save() {
  Write(NodeIndex::Name, sym_table_path);
}

void WorkbenchData::Load() {
  sym_table_path = Read(NodeIndex::Name);
}