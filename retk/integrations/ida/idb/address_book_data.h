#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "idb_store.h"

class AddressBookData final : public IdbNode {
 public:
  AddressBookData();

  struct Entry {

  };

  void Save() override;
  void Load() override;

 private:

  enum NodeIndex : nodeidx_t {
    StorageVersion,
  };
};