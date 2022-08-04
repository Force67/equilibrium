// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <new>
#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/math/alignment.h>

namespace base {
namespace {
// more than 3 would waste too many cycles
constexpr i32 kPageAcquireAttempts = 3;

bool IntersectsReasonable(mem_size given, mem_size exepected) {
  return given <= base::NextPowerOf2(exepected);
}
}  // namespace

BucketAllocator::BucketAllocator() {}

void* BucketAllocator::Allocate(PageTable& pages,
                                mem_size size,
                                mem_size user_alignment) {
   // this case should be validated by the memory router.
#if defined(BASE_MEM_CORE_DEBUG)
  // cant handle this...
  if (size > eq_allocation_constants::kBucketThreshold ||
      user_alignment > eq_allocation_constants::kBucketThreshold)
    return nullptr;
#endif

  // TODO: if our alignment is perfect by the user, maybe try to not destroy that.
  // dont destroy perfect alignment with our bucket, instead put it in some
  // free list then... but then again, a problem would be fragmentation if we would
  // be to maintain a seperate freelist :thonk:
  size += sizeof(Bucket);
  mem_size alignment = base::NextPowerOf2(size);
  if (user_alignment > 0 && user_alignment <= alignment) {
    alignment = user_alignment;
    size = base::Align(size, alignment);
  } else
    size = alignment;

  // TODO: worry about thread safety
  i32 attempts = 0;
  byte* page_hint = nullptr;
  do {
    if (void* block = AcquireMemory(size, page_hint))
      return block;
    if (!TryAcquireNewPage(pages, page_hint))
      attempts++;
  } while (attempts <= kPageAcquireAttempts);

  return nullptr;
}

void* BucketAllocator::ReAllocate(PageTable&,
                                  void* former_block,
                                  mem_size new_size,
                                  mem_size user_alignment) {
  return nullptr;
}

void* BucketAllocator::AcquireMemory(mem_size size, byte* hint) {
  if (hint) {
    // do something with the hint index
  }

  byte* page_head = nullptr;
  if (Bucket* bucket = FindFreeBucket(size, page_head)) {
    // return user memory
    return reinterpret_cast<void*>(page_head + sizeof(PageTag) + bucket->offset_);
  }
  return nullptr;
}

bool BucketAllocator::TryAcquireNewPage(PageTable& table, byte*& page_base) {
  // this needs to lock
  mem_size page_size = 0;
  page_base = static_cast<byte*>(table.RequestPage(page_size));
  if (!page_base || page_size == 0) {
    DEBUG_TRAP; // page or page size invalid
    return false;
  }
  // store the entire node in the page itself
  page_list_.Append(new (page_base) HeaderNode(page_size));
  return true;
}

i32 BucketAllocator::FindFreeBucketHead(mem_size requested_size) {
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    // manage the page refcount (to prevent the page from being deallocated)
    ScopedPageAccess _(node->value()->tag);

    PageTag& tag = node->value()->tag;

    if (tag.bucket_count == 0) {
      // atomic store, set the bucket as used
      *reinterpret_cast<AtomicBucket*>(tag.end() - sizeof(Bucket)) = Bucket();
      return 0;
    }

    for (auto i = 1; i < tag.bucket_count; i++) {
      auto bucket =
          (*reinterpret_cast<AtomicBucket*>(tag.end() - (sizeof(Bucket) * i)))
              .load();
      if (bucket.flags_ & Bucket::kReleased &&
          IntersectsReasonable(bucket.size_, requested_size)) {
        *reinterpret_cast<AtomicBucket*>(tag.end() - sizeof(Bucket)) =
            Bucket();  // claim (in ctor)
        return static_cast<i32>(tag.bucket_count++);
      }
    }

    // need to find if any buckets intersect at n
  }

  return 0;
}

bool BucketAllocator::DoAnyBucketsIntersect(const PageTag& tag) {
  const byte* page_base = reinterpret_cast<const byte*>(&tag);

#if 0
  for (auto i = 0; i < tag.bucket_count; i++) {
    auto bucket =
        (*reinterpret_cast<AtomicBucket*>(tag.end() - (sizeof(Bucket) * i))).load();
    if (bucket.IsinUse()) {
      byte* memory = DecompressPagePointer(page_base, bucket.offset_);
    }
  }
#endif
  return false;
}

// https://source.chromium.org/chromium/chromium/src/+/main:base/atomic_sequence_num.h;bpv=1;bpt=1

void BucketAllocator::TakeMemoryChunk(Bucket& bucket,
                                      uint8_t* start_hint,
                                      mem_size req_size) {
  bucket.offset_ = 0;
  bucket.flags_ = 0;
  bucket.size_ = static_cast<u16>(req_size);
}

BucketAllocator::Bucket* BucketAllocator::FindFreeBucket(mem_size requested_size,
                                                         byte*& page_start) {
#if 0
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    PageTag& tag = node->value()->tag;
    page_start = tag.begin();

    // looks like a fresh page
    if (tag.bucket_count == 0) {
      TakeMemoryChunk(*tag.bucket_table, page_start, requested_size);
      tag.bucket_count++;
      return header.bucket_table;
    }

    byte* page_end = tag.end();

    // attempt reclaiming unused buckets (and their memory, which is orphaned)
    for (auto i = 0; i < tag.bucket_count; i++) {
      Bucket* buck = reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * i));
      // reclaim previously used bucket
      //
      // TODO: should make a resonable choice, based on alignment, if we can reclaim
      // the block rather than relying on the same size...
      if (buck->flags_ == 1 && buck->size_ >= requested_size) {
        TakeMemoryChunk(*buck, page_start + sizeof(PageTag) + buck->offset_,
                        requested_size);
        tag.bucket_count++;
        return buck;
      }
    }
    // new bucket
    // todo: validate if buckets are owned by any other memory high prio!!!
    Bucket* free_bucket = reinterpret_cast<Bucket*>(
        page_end - (sizeof(Bucket) * (tag.bucket_count + 1)));
    free_bucket->flags_ = 0;
    free_bucket->size_ = static_cast<u16>(requested_size);
    // free_bucket->offset_ =

    return free_bucket;
  }
#endif
  return nullptr;
}

BucketAllocator::Bucket* BucketAllocator::FindBucket(pointer_size address) {
  return nullptr;
}

void BucketAllocator::Free(void* pointer) {
#if 0
  byte* block = reinterpret_cast<byte*>(pointer);
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    // is owned by given page?
    byte* page_start = reinterpret_cast<byte*>(node);
    byte* page_end = page_start + node->value()->header.page_size;
    if (block >= page_start && block <= page_end) {
      // now free it!

      for (auto i = 0; i < node->value()->header.bucket_count; i++) {
        Bucket* buck = reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * i));
        if (block >= (page_start + buck->offset_) &&
            block <= (page_start + buck->offset_ + buck->size_)) {
          buck->flags_ =
              1;  // TODO: a threshhold where we start decrementing the bucket count/
          // and actually provide the bucket as memory space?
          return;
        }
      }
    }
  }

  DCHECK(false, "BucketAllocator::Free(): Failed to release memory");
#endif
}
}  // namespace base