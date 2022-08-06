// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/memory_stat_tracker.h>

namespace base {
namespace {
thread_local constinit MemoryCategory current_token{kGeneralMemory};
}  // namespace

// TODO: guard lock this.
MemoryCategory AddMemoryCategory(const char* name) {
  auto& tracker_instance = memory_coordinator().tracker();

  // find a free entry.
  MemoryCategory index = kTrackingLimit + 1;
  for (auto i = 0; i < kTrackingLimit; i++) {
    auto& entry = tracker_instance.token_bucket[i];
    if (entry == kInvalidCategory)
      entry = index = i;
  }

  // TODO: handle tracking limit

  if (index < kTrackingLimit)
    tracker_instance.name_bucket[index] = name;

  return index;
}

// TODO: guard lock this.
void RemoveMemoryCategory(MemoryCategory id) {
  auto& tracker_instance = memory_coordinator().tracker();

  for (auto i = 0; i < kTrackingLimit; i++) {
    auto& entry = tracker_instance.token_bucket[i];
    if (entry == id)
      entry = kInvalidCategory;
  }
}

void MemoryTracker::TrackOperation(pointer_diff size) {
  DCHECK(current_token != kInvalidCategory, "Category not set");
  //DCHECK(pointer_diff((memory_sizes[current_token] + size) /*atomic op*/) < 0,
  //       "Underflow into tracking storage");

  memory_sizes[current_token] += size;
}

MemoryCategoryScope::MemoryCategoryScope(MemoryCategory token)
    : cur_(token), prev_(current_token) {
  if (token != current_token)
    current_token = token;
}

MemoryCategoryScope::~MemoryCategoryScope() {
  current_token = prev_;
}
}  // namespace base