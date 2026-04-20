// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/memory_stat_tracker.h>

#include <cstring>

#include <base/threading/lock_guard.h>
#include <base/threading/mutex.h>

namespace base {
namespace {
thread_local constinit MemoryCategory current_token{kGeneralMemory};
static base::Mutex s_tracker_mutex;

MemoryCategory FindFreeTokenIndex(MemoryTracker& tracker) {
  // scan slots 0..253 (254 = kGeneralMemory is reserved)
  for (int i = 0; i < static_cast<int>(kGeneralMemory); i++) {
    MemoryCategory& token_entry = tracker.token_bucket[i];
    if (token_entry == kInvalidCategory) {
      token_entry = static_cast<MemoryCategory>(i);
      return static_cast<MemoryCategory>(i);
    }
  }
  return kInvalidCategory;
}
}  // namespace

static bool s_categories_inited = false;

MemoryCategory AddMemoryCategory(const char* name) {
  base::LockGuard<base::Mutex> lock(s_tracker_mutex);

  auto& tracker_instance = memory_tracker();
  if (!s_categories_inited) {
    tracker_instance.WipeStats();
    s_categories_inited = true;
  }
  const MemoryCategory index = FindFreeTokenIndex(tracker_instance);

  if (index < kInvalidCategory)
    tracker_instance.name_bucket[index] = name;
  else
    BASE_BUGCHECK(false, "Invalid category");

  return index;
}

void RemoveMemoryCategory(MemoryCategory id) {
  base::LockGuard<base::Mutex> lock(s_tracker_mutex);

  auto& tracker_instance = memory_tracker();
  for (auto i = 0; i < kTrackingLimit; i++) {
    MemoryCategory& token_entry = tracker_instance.token_bucket[i];
    if (token_entry == id) {
      if (token_entry < kInvalidCategory) {
        tracker_instance.name_bucket[token_entry] = nullptr;
        tracker_instance.memory_sizes[token_entry] = 0u;
      } else
        BASE_BUGCHECK(false);

      token_entry = kInvalidCategory;
      break;
    }
  }
}

MemoryCategory MemoryTracker::CurrentCategory() {
  return current_token;
}

void MemoryTracker::WipeStats() {
  for (MemoryCategory i = 0; i < kTrackingLimit; i++) {
    token_bucket[i] = kInvalidCategory;
    name_bucket[i] = nullptr;
    memory_sizes[i].store(0, std::memory_order_relaxed);
  }
  // reserve the general category slot
  token_bucket[kGeneralMemory] = kGeneralMemory;
  name_bucket[kGeneralMemory] = "<general>";
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
