// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/memory_stat_tracker.h>

#include <base/threading/lock_guard.h>
#include <base/threading/spinning_mutex.h>

namespace base {
namespace {
thread_local constinit MemoryCategory current_token{kGeneralMemory};

MemoryCategory FindFreeTokenIndex(MemoryTracker& tracker) {
  MemoryCategory index{kInvalidCategory};
  for (auto i = 0; i < (kTrackingLimit - 1); i++) {
    MemoryCategory& token_entry = tracker.token_bucket[i];
    if (token_entry == kInvalidCategory) {
      token_entry = index = i;
      break;
    }
  }
  return index;
}
}  // namespace

MemoryCategory AddMemoryCategory(const char* name) {
  // lock
  base::ScopedLockGuard<base::SpinningMutex> _;
  (void)_;

  auto& tracker_instance = memory_tracker();
  const MemoryCategory index = FindFreeTokenIndex(tracker_instance);

  if (index < kInvalidCategory)
    tracker_instance.name_bucket[index] = name;
  else
    BUGCHECK(false, "Invalid category");

  return index;
}

void RemoveMemoryCategory(MemoryCategory id) {
  // lock
  base::ScopedLockGuard<base::SpinningMutex> _;
  (void)_;

  auto& tracker_instance = memory_tracker();
  for (auto i = 0; i < kTrackingLimit; i++) {
    MemoryCategory& token_entry = tracker_instance.token_bucket[i];
    if (token_entry == id) {
      if (token_entry < kInvalidCategory) {
        tracker_instance.name_bucket[token_entry] = nullptr;
        tracker_instance.memory_sizes[token_entry] = 0u;
      } else
        BUGCHECK(false);

      token_entry = kInvalidCategory;
      break;
    }
  }
}

void MemoryTracker::TrackOperation(void* pointer, pointer_diff size) {
  // DCHECK(current_token != kInvalidCategory, "Category not set");
  //  DCHECK(pointer_diff((memory_sizes[current_token] + size) /*atomic op*/) < 0,
  //         "Underflow into tracking storage");

  memory_sizes[current_token] += size;
}

MemoryCategory current_memory_category() {
  return current_token;
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