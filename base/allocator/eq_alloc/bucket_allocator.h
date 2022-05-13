// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/allocator/eq_alloc/allocator.h>

namespace base {
// https://fossies.org/linux/serf/buckets/allocator.c

// the bucketallocator operates on bits within pages,
struct BucketAllocator final : Allocator {
  virtual void* Allocate(PageTable& pageman,
                         size_t size,
                         size_t alignment = 1024) override;
  virtual void Free(void* block) override;
};
}  // namespace base