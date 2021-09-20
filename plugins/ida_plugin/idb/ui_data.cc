// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "ui_data.h"

namespace {
constexpr char kStorageNodeId[] = "$ retk_ui_data";
constexpr int kUiStorageVersion = 1;
}  // namespace

UiData::UiData() : IdbNode(kStorageNodeId) {}

bool UiData::SeenBefore() {
  return utils::NetNode::Exists(kStorageNodeId);
}

void UiData::Save() {
  bool res = Write(Tick, tick) && Write(RunIndex, last_run_index);
  LOG_TRACE("UiStorage::Save {}", res);
}

void UiData::Load() {
  int v1 = Read(StorageVersion, -1);

  if (v1 == -1) {
    bool res = Write(StorageVersion, kUiStorageVersion);
    LOG_TRACE("StoreVersion: {}", res);
  }

  // cry out loud...
  if (v1 < kUiStorageVersion) {
    LOG_WARNING("Outdated UiStorageVersion: idb: {} plugin: {}", v1,
                kUiStorageVersion);
  }

  tick = Read(Tick, 0);
  last_run_index = Read(RunIndex, 0);
  LOG_TRACE("UiStorage::Load {}", tick);
}