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

  public:
  explicit BucketAllocator(PageTable&);

  void* Allocate(mem_size size, mem_size alignment = 0) override;

  void* ReAllocate(void* former_block,
                   mem_size new_size,
                   mem_size user_alignment = 0) override;
  mem_size Free(void* block) override;

  mem_size QueryAllocationSize(void* block) override;

 private:
  void* AcquireMemory(mem_size user_size,
                      mem_size size,
                      byte* hint = nullptr);

 private:
  PageTable& page_table_;
  base::SpinningMutex lock_;

  struct BucketInfo {
    u32 offset_{0};  // offset starting from page_base

    enum Flags : u16 {
      kNone,
      kReleased = 1 << 1,
      kUsed = 1 << 2,
    };
    union Value {
      struct {
        // 11 bits to store a value up to 1024, max
        // would be 2047
        u32 user_size : 11;     // this is the allocation without alignment
        u32 aligned_size : 11;  // the actual size of the allocation
        u32 flags : 10;
      };
      u32 raw;

      Value() : raw(0) {}
      explicit Value(uint32_t val) : raw(val) {}
    };
    Value value_;

    explicit BucketInfo(u32 offset, u16 user_size, u16 size, Flags flags)
        : offset_(offset) {
      value_.user_size = user_size;
      value_.aligned_size = size;
      value_.flags = flags;
    }

    inline bool IsinUse() const { return value_.flags & Flags::kUsed; }
    inline bool IsFree() const { return value_.flags & Flags::kReleased; }
    inline void SetFree() { value_.flags = Flags::kReleased; }
    inline void SetUsed() { value_.flags = Flags::kUsed; }

    inline void SetUserSize(u32 size) { value_.user_size = size; }
    inline void SetSize(u32 size) { value_.aligned_size = size; }

    inline u32 user_size() const { return value_.user_size; }
    inline u32 size() const { return value_.aligned_size; }
  };
  static_assert(sizeof(BucketInfo) == sizeof(pointer_size),
                "Bucket is too fat");

  union BucketStore {
    BucketInfo bucket;
    pointer_size as_pointer;
  };
  using BucketPointer = base::Atomic<BucketStore>;
  using AtomicBucket = base::Atomic<BucketInfo>;

  static_assert(sizeof(BucketInfo) == sizeof(pointer_size),
                "Bucket must fit into atomic/register space");

  BucketInfo* FindBucket(pointer_size address);

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
    mem_size page_size;  // not really needed atm since we know the size is
                         // always 65k, but if we wanna go for a hybrid model,
                         // it might be worth it
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

  // void TakeMemoryChunk(Bucket&, uint8_t* start_hint, mem_size req_size);
  BucketInfo* FindAndClaimFreeBucket(mem_size actual_size,
                                     mem_size aligned_size,
                                     byte*&);
};
}  // namespace base