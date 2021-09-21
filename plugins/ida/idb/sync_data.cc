// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "sync_data.h"

namespace {
constexpr char kSyncDataName[] = "$ retk_sync_data";
constexpr int kSessionStoreVersion = 1;
}

SyncData::SyncData() : IdbNode(kSyncDataName) {

}

void SyncData::Load() {
  int version = Read(NodeIndex::StorageVersion, -1);
  if (version < kSessionStoreVersion) {
    // apply change set..
  }

  // no node exists yet..
  if (version == -1) {
    Write(NodeIndex::StorageVersion, kSessionStoreVersion);
  }

  version_ = Read(NodeIndex::SessionVersion, 0);
}

void SyncData::Save() {
  // attempt to rescue the node
  if (!IsOpen()) {
    Load();
  }

  bool res;
  res = Write(NodeIndex::SessionVersion, version_);
}