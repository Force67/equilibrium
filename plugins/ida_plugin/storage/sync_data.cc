// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "sync_data.h"

namespace {
constexpr char kSyncDataName[] = "$ retk_sync_data";
constexpr int kSessionStoreVersion = 1;
}

const char* const SyncData::GetName() {
  return kSyncDataName;
}

void SyncData::Load() {
  _node = noda::NetNode(kSyncDataName);

  int version = _node.LoadScalar<int>(NodeIndex::StorageVersion, -1);
  if (version < kSessionStoreVersion) {
    // apply change set..
  }

  // no node exists yet..
  if (version == -1) {
    _node.StoreScalar(NodeIndex::StorageVersion, kSessionStoreVersion);
  }

  version_ = _node.LoadScalar<int>(NodeIndex::SessionVersion, 0);
}

void SyncData::Save() {
  // attempt to rescue the node
  if (!_node.open()) {
    Load();
  }

  bool res;
  res = _node.StoreScalar(NodeIndex::SessionVersion, version_);
}