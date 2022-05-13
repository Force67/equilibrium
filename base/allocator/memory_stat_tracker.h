// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <atomic>
#include <base/arch.h>

namespace base {

// memory categories are used to label specific allocations
// e.g a LoadTexture function that tags all allocations within it as
// 'TextureMemory'
using MemoryCategory = u16;

constexpr MemoryCategory kTrackingLimit = 256;

constexpr MemoryCategory kInvalidCategory{ku16Bounds.max};
constexpr MemoryCategory kGeneralMemory = 0;

struct MemoryTracker {
  // 0xfff... means that the entry is unused,
  // 0 means that we fall under the general category,
  // e.g noname
  MemoryCategory token_bucket[kTrackingLimit]{ku16Bounds.max};
  // token indicies show us the name index we need
  const char* name_bucket[kTrackingLimit]{nullptr};
  std::atomic<size_t> memory_sizes[kTrackingLimit]{0};

  void TrackAllocation(size_t size);
  void TrackDeallocation(size_t size);
};

STRONG_INLINE void SetMemoryTrackerInstance(MemoryTracker*);

// this returns a token that is used for labeling that specific memory
// make sure to retain ownership of your name param
MemoryCategory AddMemoryCategory(const char* name);
void RemoveMemoryCategory(MemoryCategory token);

class MemoryCategoryScope {
 public:
  explicit MemoryCategoryScope(MemoryCategory token);
  ~MemoryCategoryScope();

 private:
  MemoryCategory prev_;
  MemoryCategory cur_;
};
}  // namespace base