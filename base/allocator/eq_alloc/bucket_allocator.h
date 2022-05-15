// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/allocator/eq_alloc/allocator.h>

namespace base {
// https://fossies.org/linux/serf/buckets/allocator.c

// the bucketallocator operates on bits within pages,
class BucketAllocator final : public Allocator {
 public:
  virtual void* Allocate(PageTable& pageman,
                         size_t size,
                         size_t alignment = 1024) override;
  virtual void Free(void* block) override;

 private:
  // since this is a small bucket allocator (allocations always guaranteed to be <
  // 1024 bytes) we can just store the size in a half
  struct Bucket {
    u16 compressed_address_{0};
    u16 size_{0};
  };
  Bucket* FindFreeBucket();

  // list of base pointers(one for each page)
  pointer_size* base_pointers_;
};
}  // namespace base