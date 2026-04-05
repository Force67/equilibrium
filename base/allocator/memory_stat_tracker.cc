// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <base/allocator/memory_coordinator.h>
#include <base/allocator/memory_stat_tracker.h>

#include <cstring>
#include <mutex>

namespace base {
namespace {
thread_local constinit MemoryCategory current_token{kGeneralMemory};
static std::mutex s_tracker_mutex;

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
  std::lock_guard<std::mutex> lock(s_tracker_mutex);

  auto& tracker_instance = memory_tracker();
  const MemoryCategory index = FindFreeTokenIndex(tracker_instance);

  if (index < kInvalidCategory)
    tracker_instance.name_bucket[index] = name;
  else
    BASE_BUGCHECK(false, "Invalid category");

  return index;
}

void RemoveMemoryCategory(MemoryCategory id) {
  std::lock_guard<std::mutex> lock(s_tracker_mutex);

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

static constinit bool HACK_INITED{false};

void MemoryTracker::TrackOperation(void* /*pointer*/, pointer_diff size) {
  if (!HACK_INITED) {
    WipeStats();
    HACK_INITED = true;
  }
  memory_sizes[current_token].fetch_add(size);
}

void MemoryTracker::WipeStats() {
  for (MemoryCategory i = 0; i < kTrackingLimit; i++) {
    token_bucket[i] = kInvalidCategory;
    name_bucket[i] = nullptr;
    memory_sizes[i].store(0, std::memory_order_relaxed);
  }
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
