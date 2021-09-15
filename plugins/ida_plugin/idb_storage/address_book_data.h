#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "data_store.h"

class AddressBookData final : public NodeTraits {
 public:
  struct Entry {

  };

  void Save() override;
  void Load() override;

  const char* const GetName() override;

 private:

  enum NodeIndex : nodeidx_t {
    StorageVersion,
  };
};