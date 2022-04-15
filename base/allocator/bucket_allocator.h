// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/memory_coordinator.h>

namespace base {
// https://fossies.org/linux/serf/buckets/allocator.c
struct BucketAllocator final : Allocator {
  constexpr virtual void* Allocate(size_t size, size_t alignment = 1024) override;
  constexpr virtual void Free(void* block) override;
};
}