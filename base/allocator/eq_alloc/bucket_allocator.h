// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <atomic>
#include <base/arch.h>
#include <base/containers/linked_list.h>
#include <base/allocator/eq_alloc/allocator.h>

namespace base {
// https://fossies.org/linux/serf/buckets/allocator.c

// the bucketallocator operates on bits within pages,
class BucketAllocator final : public Allocator {
 public:
  BucketAllocator();

  virtual void* Allocate(PageTable& pageman,
                         size_t size,
                         size_t alignment = 1024) override;
  virtual void Free(void* block) override;

  private:
  void* AcquireMemory(mem_size size);

 private:
  // buckets are stored in a growlist at the end of the page
  struct Bucket {
    u32 offset_{0};
    u16 size_{0};  // doesnt help much with alignment tho
    u16 flags_{0};
  };

  struct PageHeader {
    std::atomic<mem_size> bucket_count{0};
    size_t page_size;
    Bucket* bucket_table{};

    byte* GetData() {
      return reinterpret_cast<byte*>(this) + sizeof(PageHeader);
    }
  };
  class HeaderNode : public LinkNode<HeaderNode> {
   public:
    PageHeader header;
  };
  LinkedList<HeaderNode> page_list_;

  bool TryAcquireNewPage(PageTable&);

  byte* DecompressPagePointer(pointer_size page_base, u16 offset) {
    return reinterpret_cast<byte*>(page_base + offset);
  }

  void TakeMemoryChunk(Bucket&, uint8_t* start_hint, mem_size req_size);
  Bucket* FindFreeBucket(mem_size requested_size, byte*&);
};
}  // namespace base