// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/atomic.h>
#include <base/containers/linked_list.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/threading/spinning_mutex.h>

namespace base {
// https://fossies.org/linux/serf/buckets/allocator.c

// the bucketallocator operates on bits within pages,
class BucketAllocator final : public Allocator {
  friend struct EQMemoryRouter;

 protected:
  explicit BucketAllocator(PageTable&);

  void* Allocate(mem_size size, mem_size alignment = 0) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 0) override;
  bool Free(void* block) override;

 private:
  void* AcquireMemory(mem_size size, byte* hint = nullptr);

 private:
  PageTable& page_table_;
  base::SpinningMutex lock_;

  struct Bucket {
    u32 offset_{0};  // offset starting from page_base
    u16 size_{0};

    enum Flags : u16 {
      kNone,
      kReleased = 1 << 1,
      kUsed = 1 << 2,
    };
    u16 flags_{kUsed};

    explicit Bucket(u32 offset, u16 size, Flags flags)
        : offset_(offset), size_(size), flags_(flags) {}

    inline bool IsinUse() const { return flags_ & Flags::kUsed; }
  };
  union BucketStore {
    Bucket bucket;
    pointer_size as_pointer;
  };
  using BucketPointer = base::Atomic<BucketStore>;
  using AtomicBucket = base::Atomic<Bucket>;

  static_assert(sizeof(Bucket) == sizeof(pointer_size),
                "Bucket must fit into atomic/register space");

  Bucket* FindBucket(pointer_size address);

  // (Bucket) Page Memory layout:
  // +-------------------------------------------------------------------+
  // |                             PageTag                               |
  // |   +-------------+--------------+------------------+               |
  // |   |  ref_count  | bucket_count |  page_size       |               |
  // |   +-------------+--------------+------------------+               |
  // |                                                                   |
  // |                           HeaderNode                              |
  // |   +-------------------------------------------------------+       |
  // |   |  tag (ref_count, bucket_count, page_size)            |        |
  // |   +-------------------------------------------------------+       |
  // |                                                                   |
  // |                           Usable Data                             |
  // |   +------------------+------------------+----                     |
  // |   |                  |                  |    ...                  |
  // |   +------------------+------------------+----                     |
  // |                                                                   |
  // |                         Bucket Metadata                           |
  // |   +------------------+------------------+----                     |
  // |   |    Bucket 1      |    Bucket 2      |    ...                  |
  // |   +------------------+------------------+----                     |
  // +-------------------------------------------------------------------+

  struct PageTag {
    base::Atomic<mem_size> ref_count;  // TODO: impl it
    base::Atomic<mem_size> bucket_count;
    mem_size
        page_size;  // not really needed atm since we know the size is always 65k,
                    // but if we wanna go for a hybrid model, it might be worth it
                    //

    // better dont ask. we need to do it since we loose 16 bytes to our ancestor
    // aswell
    byte* begin() const {
      return const_cast<byte*>(reinterpret_cast<const byte*>(this) -
                               sizeof(base::internal::LinkNodeBase));
    }
    byte* end() const { return begin() + page_size; }

    // this returns the beginning of actually useable data...
    byte* data() const { return begin() + sizeof(HeaderNode); }
  };
  static_assert(sizeof(PageTag) == 24, "Pagetag is of a weird size");

  bool DoAnyBucketsIntersect(const PageTag& tag);

  class HeaderNode final : public LinkNode<HeaderNode> {
   public:
    explicit HeaderNode(mem_size page_size) : tag{0, 0, page_size} {}

    PageTag tag;
  };
  static_assert(sizeof(HeaderNode) == 40, "HeaderNode is of a weird size");

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