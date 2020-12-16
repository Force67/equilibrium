// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UiStorage.h"
#include "Pch.h"
#include "utils/Logger.h"

namespace {
// unique identifier for IDB storage
// please do *not* change these
constexpr char kStorageNodeId[] = "$ noda_ui_storage";

// increment this whenever a breaking change happens
constexpr int kUiStorageVersion = 1;

enum NodeIndex : nodeidx_t { StorageVersion, Tick, IdbUsageFlags };
}  // namespace

void UiStorage::SetTick(uint32_t newVal) {
  _tick = newVal;
}

int UiStorage::GetTick() const {
  return _tick;
}

bool UiStorage::SeenBefore() {
  return noda::NetNode(kStorageNodeId, false).open();
}

void UiStorage::Load() {
  _node = noda::NetNode(kStorageNodeId);

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

  _tick = _node.LoadScalar(Tick, 0);
  LOG_TRACE("UiStorage::Load {}", _tick);
}

void UiStorage::Save() {
  bool res;
  res = _node.StoreScalar(Tick, _tick);

  LOG_TRACE("UiStorage::Save {}", res);
}