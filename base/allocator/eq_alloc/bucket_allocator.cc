// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <new>
#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/math/alignment.h>

#include <base/threading/lock_guard.h>

namespace base {
namespace {
// more than 3 would waste too many cycles
constexpr i32 kPageAcquireAttempts = 3;

bool IntersectsReasonable(mem_size given, mem_size exepected) {
  return given <= base::NextPowerOf2(exepected);
}
}  // namespace

BucketAllocator::BucketAllocator(PageTable& t) : page_table_(t) {
  byte* page_hint = nullptr;
  DCHECK(TryAcquireNewPage(page_table_, page_hint),
         "No initial page for the bucket allocator to work with!");
}

void* BucketAllocator::Allocate(mem_size size, mem_size alignment) {
  // this case should be validated by the memory router.
#if defined(BASE_MEM_CORE_DEBUG)
  // cant handle this...
  if (size > eq_allocation_constants::kBucketThreshold ||
      user_alignment > eq_allocation_constants::kBucketThreshold)
    return nullptr;
#endif
#if 0
  if ((alignment & (alignment - 1)) != 0) {
    // alignment is not a power of 2
    DEBUG_TRAP;
    return nullptr;
  }
#endif

  // align to a sensible boundary
  size += sizeof(Bucket);
  size = base::Align(size, alignment);

  {
    base::NonOwningScopedLockGuard _(lock_);
    (void)_;

    i32 attempts = 0;
    byte* page_hint = nullptr;
    do {
      if (void* block = AcquireMemory(size, page_hint))
        return block;
      if (!TryAcquireNewPage(page_table_, page_hint))
        attempts++;
    } while (attempts <= kPageAcquireAttempts);
  }
  return nullptr;
}

void* BucketAllocator::ReAllocate(void* former_block,
                                  mem_size new_size,
                                  mem_size user_alignment) {
  if (former_block == nullptr) {
    // if former_block is null, then it behaves the same as Allocate
    return Allocate(new_size, user_alignment);
  }

  if (new_size == 0) {
    // if new_size is zero, then it behaves the same as Free
    Free(former_block);
    return nullptr;
  }

  pointer_size former_address = reinterpret_cast<pointer_size>(former_block);
  Bucket* former_bucket = FindBucket(former_address);

  if (former_bucket == nullptr) {
    // The former block wasn't allocated by this allocator
    return nullptr;
  }

  mem_size former_size = former_bucket->size_;

  if (former_size >= new_size) {
    // If the former block is large enough, simply return it
    return former_block;
  }

  // Otherwise, we need to allocate a new block and copy data from the former block
  void* new_block = Allocate(new_size, user_alignment);

  if (new_block == nullptr) {
    // Failed to allocate memory
    // Maybe log an error here
    DEBUG_TRAP;
    return former_block;  // Return the former_block to avoid losing it
  }

  // Copy data from the former block to the new block
  memcpy(new_block, former_block, former_size);

  // Free the former block
  Free(former_block);

  return new_block;
}

void* BucketAllocator::AcquireMemory(mem_size size, byte* hint) {
  // DCHECK(!lock_.held());

  if (hint) {
    // do something with the hint index
  }

  byte* page_head = nullptr;
  if (Bucket* bucket = FindFreeBucket(size, page_head)) {
    // return user memory
    return reinterpret_cast<void*>((page_head + sizeof(HeaderNode)) +
                                   bucket->offset_);
  }
  return nullptr;
}

bool BucketAllocator::TryAcquireNewPage(PageTable& table, byte*& page_base) {
  // this needs to lock
  mem_size page_size = 0;
  page_base = static_cast<byte*>(
      table.RequestPage(base::PageProtectionFlags::RW, &page_size));
  if (!page_base || page_size == 0) {
    DEBUG_TRAP;  // page or page size invalid
    return false;
  }
  // store the entire node in the page itself
  page_list_.Append(new (page_base) HeaderNode(page_size));
  return true;
}

bool BucketAllocator::DoAnyBucketsIntersect(const PageTag& tag) {
  // The base of the page, used to compute the physical addresses of buckets
  const byte* page_base = reinterpret_cast<const byte*>(&tag);

  // Track the end of the previous bucket in the loop
  u32 prev_end_offset = 0;

  for (auto i = 0; i < tag.bucket_count.load(); i++) {
    const auto bucket =
        (*reinterpret_cast<AtomicBucket*>(tag.end() - (sizeof(Bucket) * (i + 1))))
            .load();

    if (bucket.IsinUse()) {
      if (bucket.offset_ < prev_end_offset) {
        // The current bucket starts before the previous one ends, so they intersect
        return true;
      }
      // Update the end of the previous bucket for the next iteration
      prev_end_offset = bucket.offset_ + bucket.size_;
    }
  }

  return false;
}

// https://source.chromium.org/chromium/chromium/src/+/main:base/atomic_sequence_num.h;bpv=1;bpt=1

void BucketAllocator::TakeMemoryChunk(Bucket& bucket,
                                      uint8_t* start_hint,
                                      mem_size req_size) {
  bucket.offset_ = 0;
  bucket.flags_ = Bucket::kUsed;
  bucket.size_ = static_cast<u16>(req_size);
}

BucketAllocator::Bucket* BucketAllocator::FindFreeBucket(mem_size requested_size,
                                                         byte*& page_start) {
  for (base::LinkNode<HeaderNode>* node = page_list_.head();
       node != page_list_.end(); node = node->next()) {
    PageTag& tag = node->value()->tag;

    page_start = tag.begin();
    byte* page_end = tag.end();

    // best case, we can reclaim a previously reserved bucket, and its associated
    // memory.
    byte* data_start = tag.data();
    for (mem_size i = 0; i < tag.bucket_count; i++) {
      Bucket* buck =
          reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * (i + 1)));
      if ((buck->flags_ & Bucket::kReleased) && buck->size_ >= requested_size) {
        buck->offset_ =
            buck->offset_;  // ye i know, we might be smarter to realloc.. idk
        buck->flags_ = Bucket::kUsed;
        buck->size_ = static_cast<u16>(requested_size);
        return buck;
      }
    }

    // fastpath: If no buckets exist yet, create the first one at the beginning of
    // the available data space
    if (tag.bucket_count == 0) {
      Bucket* free_bucket =
          new (page_end - (sizeof(Bucket) * (node->value()->tag.bucket_count + 1)))
              Bucket(/*offset*/ 0, /*size*/ requested_size, /*flags*/ Bucket::kUsed);
      node->value()->tag.bucket_count++;
      return free_bucket;
    }

    // TODO: refactor this to be more efficient

    // find the largest gap between two buckets...
    mem_size max_free_space = 0;
    mem_size offset = 0;
    byte* last_bucket_end = data_start;
    for (mem_size i = 0; i < tag.bucket_count; i++) {
      Bucket* meta_data =
          reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * (i + 1)));

      byte* bucket_data_start = DecompressPagePointer(
          reinterpret_cast<pointer_size>(data_start), meta_data->offset_);
      byte* bucket_data_end = bucket_data_start + meta_data->size_;

      mem_size free_space = bucket_data_start - bucket_data_end;
      if (free_space > max_free_space) {
        max_free_space = free_space;
        offset = last_bucket_end - data_start;
      }
      last_bucket_end = bucket_data_end;
    }

    // Check if we found a large enough gap
    if (max_free_space >= requested_size) {
      // We did, create a bucket in the gap
      Bucket* free_bucket =
          new (page_end - (sizeof(Bucket) * (node->value()->tag.bucket_count + 1)))
              Bucket(/*offset*/ offset, /*size*/ requested_size,
                     /*flags*/ Bucket::kUsed);
      node->value()->tag.bucket_count++;
      return free_bucket;
    }

    // Check if we can place a bucket at the end of the last bucket

    // this is not super smart, we put the end directly below the last metadataentry
    byte* data_end =
        page_end - (sizeof(Bucket) * (node->value()->tag.bucket_count + 1));
    // get the last metadata entry:
    Bucket* last_buck = reinterpret_cast<Bucket*>(
        page_end - (sizeof(Bucket) * (node->value()->tag.bucket_count)));
    if ((data_end - ((data_start + last_buck->offset_ + last_buck->size_))) <
        requested_size) {
      DEBUG_TRAP;
      return nullptr;
    } 

    offset = last_buck->offset_ + last_buck->size_;
    Bucket* free_bucket =
        new (data_end)
            Bucket(/*offset*/ offset, /*size*/ requested_size,
                   /*flags*/ Bucket::kUsed);
    node->value()->tag.bucket_count++;
    return free_bucket;
  }
  return nullptr;
}

BucketAllocator::Bucket* BucketAllocator::FindBucket(pointer_size address) {
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    byte* page_start = reinterpret_cast<byte*>(node);
    byte* page_end = page_start + node->value()->tag.page_size;
    if (address >= reinterpret_cast<pointer_size>(page_start) &&
        address < reinterpret_cast<pointer_size>(page_end)) {
      for (auto i = 0; i < node->value()->tag.bucket_count; i++) {
        Bucket* buck =
            reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * (i + 1)));
        if (address >=
                (reinterpret_cast<pointer_size>(page_start) + buck->offset_) &&
            address < (reinterpret_cast<pointer_size>(page_start) + buck->offset_ +
                       buck->size_)) {
          return buck;
        }
      }
    }
  }
  return nullptr;
}

bool BucketAllocator::Free(void* pointer) {
  byte* block = reinterpret_cast<byte*>(pointer);
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    byte* page_start = reinterpret_cast<byte*>(node);
    byte* page_end = page_start + node->value()->tag.page_size;
    if (block >= page_start && block <= page_end) {
      for (auto i = 0; i < node->value()->tag.bucket_count; i++) {
        Bucket* buck = reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * i));
        if (block >= (page_start + buck->offset_) &&
            block <= (page_start + buck->offset_ + buck->size_)) {
          buck->flags_ = Bucket::kReleased;
          return true;
        }
      }
    }
  }
  DCHECK(false, "BucketAllocator::Free(): Failed to release memory");
  return false;
}
}  // namespace base