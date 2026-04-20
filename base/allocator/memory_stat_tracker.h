// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/atomic.h>
#include <base/compiler.h>
#include <base/export.h>
#include <base/numeric_limits.h>

namespace base {

// memory categories are used to label specific allocations
// e.g a LoadTexture function that tags all allocations within it as
// 'TextureMemory'
using MemoryCategory = u8;

constexpr MemoryCategory kTrackingLimit = 255;
constexpr MemoryCategory kGeneralMemory{base::MinMax<MemoryCategory>::max() - 1};
constexpr MemoryCategory kInvalidCategory{kTrackingLimit};

// this is not a proper class since we want to allow constinit for the MC
struct BASE_EXPORT MemoryTracker {
  MemoryTracker() {};

  // hot path: called on every alloc/free through the memory coordinator.
  // must be inline so the compiler can fold it into the caller.
  inline void TrackOperation(void* pointer, pointer_diff size) {
    (void)pointer;
    memory_sizes[CurrentCategory()].fetch_add(size, base::memory_order_relaxed);
  }

  // thread-local category accessor (defined in .cc, declared here for inline use)
  static MemoryCategory CurrentCategory();

  void WipeStats();

  // 0xfff... means that the entry is unused,
  // 0 means that we fall under the general category,
  // e.g noname

  // token_bucket[i] == kInvalidCategory means slot i is free.
  // NOTE: can't brace-init to kInvalidCategory here (only sets [0]),
  // so WipeStats() must be called before use.
  MemoryCategory token_bucket[kTrackingLimit]{};
  const char* name_bucket[kTrackingLimit]{};
  base::Atomic<mem_size> memory_sizes[kTrackingLimit]{};
};

BASE_EXPORT MemoryCategory current_memory_category();

STRONG_INLINE void SetMemoryTrackerInstance(MemoryTracker*);

// this returns a token that is used for labeling that specific memory
// make sure to retain ownership of your name param
BASE_EXPORT MemoryCategory AddMemoryCategory(const char* name);
BASE_EXPORT void RemoveMemoryCategory(MemoryCategory token);

class BASE_EXPORT MemoryCategoryScope {
 public:
  explicit MemoryCategoryScope(MemoryCategory token);
  ~MemoryCategoryScope();

 private:
  MemoryCategory prev_;
  MemoryCategory cur_;
};
}  // namespace base