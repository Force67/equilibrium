// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "data_store.h"

class UiData final : public NodeTraits {
 public:
  void Save() override;
  void Load() override;

  const char* const GetName() override;

  static bool SeenBefore();
 private:
  enum NodeIndex : nodeidx_t 
  { 
	  StorageVersion, 
	  Tick, 
	  IdbUsageFlags
  };

 public:
  uint32_t tick_;
};