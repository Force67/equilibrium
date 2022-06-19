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
  friend struct EQMemoryRouter;

 protected:
  BucketAllocator();

  void* Allocate(PageTable& pageman, mem_size size, mem_size alignment = 0) override;

  void* ReAllocate(PageTable&,
                   void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 0) override;
  void Free(void* block) override;

 private:
  void* AcquireMemory(mem_size size, byte* hint = nullptr);

 private:
  struct Bucket {
    u32 offset_{0};  // offset starting from page_base
    u16 size_{0};

    enum Flags : u16 {
      kNone,
      kReleased = 1 << 1,
      kUsed = 1 << 2,
    };
    u16 flags_{kUsed};

    inline bool IsinUse() const { return flags_ & Flags::kUsed; }
  };
  union BucketStore {
    Bucket bucket;
    pointer_size as_pointer;
  };
  using BucketPointer = std::atomic<BucketStore>;
  using AtomicBucket = std::atomic<Bucket>;

  static_assert(sizeof(Bucket) == sizeof(pointer_size),
                "Bucket must fit into atomic/register space");

  Bucket* FindBucket(pointer_size address);
  i32 FindFreeBucketHead(mem_size requested_size);

  struct PageTag {
    std::atomic<mem_size> ref_count;
    std::atomic<mem_size> bucket_count;
    mem_size page_size;  // not really needed atm, but if we wanna go for a hybrid
                         // model, it might be worth it
    // TODO: some refcount?
    byte* begin() { return reinterpret_cast<byte*>(this); }
    byte* data() { return begin() + sizeof(PageTag); }
    byte* end() { return begin() + page_size; }
  };

  bool DoAnyBucketsIntersect(const PageTag& tag);

  class HeaderNode : public LinkNode<HeaderNode> {
   public:
    explicit HeaderNode(mem_size page_size) : tag{0, 0, page_size} {}

    PageTag tag;
  };
  LinkedList<HeaderNode> page_list_;

  struct ScopedPageAccess {
    explicit ScopedPageAccess(PageTag& tag) : tag_(tag) { tag.ref_count++; }
    ~ScopedPageAccess() { tag_.ref_count--; }

    PageTag& tag_;
  };

  bool TryAcquireNewPage(PageTable&, byte*& out);

  byte* DecompressPagePointer(pointer_size page_base, u16 offset) {
    return reinterpret_cast<byte*>(page_base + offset);
  }

  void TakeMemoryChunk(Bucket&, uint8_t* start_hint, mem_size req_size);
  Bucket* FindFreeBucket(mem_size requested_size, byte*&);
};
}  // namespace base