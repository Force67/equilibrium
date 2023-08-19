// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/atomic.h>
#include <base/compiler.h>
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
struct MemoryTracker {
  MemoryTracker() { DEBUG_TRAP; };

  // since a negative complement gets added with a + anyway, we simply only ever add
  void TrackOperation(void* pointer, pointer_diff size /*signed number*/);

  void WipeStats();

  // 0xfff... means that the entry is unused,
  // 0 means that we fall under the general category,
  // e.g noname

  // is this even smart, or can we use a bitset?
  MemoryCategory token_bucket[kTrackingLimit]{
      kInvalidCategory};  // wouldnd a bitset suffice, as the index would indicate
                          // the offset?
  const char* name_bucket[kTrackingLimit]{"<noname>"};
  base::Atomic<mem_size> memory_sizes[kTrackingLimit]{0};
};

MemoryCategory current_memory_category(); 

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