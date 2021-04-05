// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "ui_data.h"

namespace {
constexpr char kStorageNodeId[] = "$ retk_ui_data";
constexpr int kUiStorageVersion = 1;
}

const char* const UiData::GetName() {
  return kStorageNodeId;
}

bool UiData::SeenBefore() {
  return utils::NetNode(kStorageNodeId, false).open();
}

void UiData::Save() {
  bool res;
  res = _node.StoreScalar(Tick, tick_);

  LOG_TRACE("UiStorage::Save {}", res);
}

void UiData::Load() {
  _node = utils::NetNode(kStorageNodeId);

  int v1 = _node.LoadScalar(StorageVersion, -1);
  // mark the node version
  if (v1 == -1) {
    bool res = _node.StoreScalar(StorageVersion, kUiStorageVersion);
    LOG_TRACE("StoreVersion: {}", res);
  }

  // cry out loud...
  if (v1 < kUiStorageVersion) {
    LOG_WARNING("Outdated UiStorageVersion: idb: {} plugin: {}", v1,
                kUiStorageVersion);
  }

  tick_ = _node.LoadScalar(Tick, 0);
  LOG_TRACE("UiStorage::Load {}", tick_);
}