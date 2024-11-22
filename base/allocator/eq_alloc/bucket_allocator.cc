// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <new>
#include <cstring>
#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>
#include <base/math/alignment.h>
#include <base/math/value_bounds.h>

#include <base/threading/lock_guard.h>

namespace base {
namespace {
// more than 3 would waste too many cycles
constexpr i32 kPageAcquireAttempts = 3;

bool IsValidAllocation(mem_size size, mem_size alignment) {
  return size <= eq_allocation_constants::kBucketThreshold &&
         alignment <= eq_allocation_constants::kBucketThreshold &&
         (alignment & (alignment - 1)) == 0;
}

// align to pointer size which is next multiple of 8, so
// 5 -> 8
// 8 -> 8
// 9 -> 16
// and so on
inline constexpr mem_size CalculateAlignedSize(mem_size size) {
  constexpr auto alignment_size = sizeof(pointer_size) - 1;
  return (size + alignment_size) & ~alignment_size;
}
}  // namespace

BucketAllocator::BucketAllocator(PageTable& t) : page_table_(t) {
  byte* page_hint = nullptr;
  DCHECK(TryAcquireNewPage(page_table_, page_hint),
         "No initial page for the bucket allocator to work with!");
}

void* BucketAllocator::Allocate(mem_size requested_size, mem_size alignment) {
  // this case should be validated by the memory router.
#if defined(BASE_MEM_CORE_DEBUG)
  if (!IsValidAllocation(requested_size, alignment)) {
    DEBUG_TRAP;
    return nullptr;
  }
#endif

  const auto aligned_size = CalculateAlignedSize(requested_size);
  {
    i32 attempts = 0;
    byte* page_hint = nullptr;
    base::NonOwningScopedLockGuard _(lock_);
    (void)_;
    do {
      if (void* block = AcquireMemory(requested_size, aligned_size))
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
  if (former_block == nullptr)
    return Allocate(new_size, user_alignment);
  if (new_size == 0) {
    Free(former_block);
    return nullptr;
  }

  pointer_size former_address = reinterpret_cast<pointer_size>(former_block);
  BucketInfo* former_bucket = FindBucket(former_address);
  if (former_bucket == nullptr) {
    // The former block wasn't allocated by this allocator
    return nullptr;
  }
  // in case the size was shrunk reuse the former block, but update the tracking
  // info
  const mem_size former_size = former_bucket->size();
  if (former_size >= new_size) {
    former_bucket->SetUserSize(
        new_size);  // only update the user size as the real size is needed to
                    // release the whole underlying memory eventually.
    return former_block;
  }
  // however.. if the size was increased, we can reuse the bucket metadata
  // entry, but we need to adjust it and allocate a new space for it in the data
  // area, so we cant just use Allocate() and Free()
  const auto aligned_new_size = CalculateAlignedSize(new_size);

  // Try to find space in the current page
  byte* page_head = nullptr;
  byte* page_end = nullptr;
  for (auto* node = page_list_.head(); node != page_list_.end(); node = node->next()) {
    PageTag& tag = node->value()->tag;
    page_head = tag.begin();
    page_end = tag.end();

    if (reinterpret_cast<byte*>(former_block) >= page_head &&
        reinterpret_cast<byte*>(former_block) < page_end) {
      // Found the page containing the former block
      byte* new_block = nullptr;

      // Check if there's enough space after the current block in the same page
      if (page_end - (reinterpret_cast<byte*>(former_block) + aligned_new_size) >=
          sizeof(BucketInfo)) {
        new_block = reinterpret_cast<byte*>(former_block);
        former_bucket->SetUserSize(new_size);      // Update the user size
        former_bucket->SetSize(aligned_new_size);  // Update the actual size
        return new_block;
      }

      // If not enough space in the current page, allocate a new block in a new
      // page
      byte* page_hint = nullptr;
      if (!TryAcquireNewPage(page_table_, page_hint)) {
        return nullptr;
      }

      new_block = reinterpret_cast<byte*>(page_hint);
      std::memcpy(new_block, former_block,
                  former_size);  // Copy data to the new block

      // Update the metadata for the new block
      BucketInfo* new_bucket =
          new (page_hint +
               static_cast<pointer_size>(page_end - page_head - sizeof(BucketInfo)))
              BucketInfo(
                  /*offset*/ 0, new_size, aligned_new_size, BucketInfo::kUsed);
      new_bucket->SetUserSize(new_size);

      // Free the old block
      former_bucket->SetFree();

      return new_block;
    }
  }

  return nullptr;
}

void* BucketAllocator::AcquireMemory(mem_size user_size, mem_size size, byte* hint) {
  // DCHECK(!lock_.held());

  if (hint) {
    // do something with the hint index
  }

  byte* page_head = nullptr;
  if (BucketInfo* bucket = FindAndClaimFreeBucket(user_size, size, page_head)) {
    // return user memory
    return reinterpret_cast<void*>((page_head + sizeof(HeaderNode)) + bucket->offset_);
  }
  return nullptr;
}

bool BucketAllocator::TryAcquireNewPage(PageTable& table, byte*& page_base) {
  // this needs to lock
  mem_size page_size = 0;
  page_base =
      static_cast<byte*>(table.RequestPage(base::PageProtectionFlags::RW, &page_size));
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
  const byte* page_base = tag.begin();

  // Track the end of the previous bucket in the loop
  u32 prev_end_offset = 0;

  for (u32 i = 0; i < tag.bucket_count.load(); i++) {
    const auto bucket =
        (*reinterpret_cast<AtomicBucket*>(tag.end() - (sizeof(BucketInfo) * (i + 1))))
            .load();

    if (bucket.IsinUse()) {
      if (bucket.offset_ < prev_end_offset) {
        // The current bucket starts before the previous one ends, so they
        // intersect
        return true;
      }
      // Update the end of the previous bucket for the next iteration
      prev_end_offset = bucket.offset_ + bucket.size();
    }
  }

  return false;
}

// https://source.chromium.org/chromium/chromium/src/+/main:base/atomic_sequence_num.h;bpv=1;bpt=1

// Location of Buckets in Memory
// Memory Layout: In each page managed by the allocator, the memory layout
// typically follows this structure:
// - Header: Contains metadata about the page, such as a list of buckets and
// other management information.
// - Data Area: The area where the actual data for each bucket is stored.
// - Bucket Metadata: Information about each bucket, stored at the end of the
// page.

// Bucket Metadata Placement: The metadata for each bucket is placed at the end
// of the page and grows upwards (towards the lower memory addresses). This is
// achieved by reserving space at the end of the memory page for Bucket
// structures, which contain metadata about each allocated bucket.

// Growth of Buckets
// New Allocations: When a new bucket is allocated, the allocator either reuses
// a free bucket (if available) or creates a new one at the end of the current
// list of bucket metadata. Offset Calculation: For a new bucket, the offset is
// determined based on the end of the data area of the last bucket. The
// allocator calculates the new start offset as last_buck->offset_ +
// last_buck->size().

// Data Placement
// Data Area: The actual data for each bucket is placed in the data area of the
// page. The data area grows downwards (towards higher memory addresses) from
// the start of the page. Offset Management: Each bucket keeps track of its data
// offset from the start of the data area. This offset is used to determine
// where the data for that bucket is stored.
BucketAllocator::BucketInfo* BucketAllocator::FindAndClaimFreeBucket(
    mem_size user_size,
    mem_size size, /* aligned size here is the full size */
    byte*& page_start) {
  constexpr float kSizeTolerance = 0.25f;  // Tolerance for size difference (25%)

  for (base::LinkNode<HeaderNode>* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    PageTag& tag = node->value()->tag;

    page_start = tag.begin();
    byte* page_end = tag.end();

    // Check for existing free bucket that can accommodate the size and is not
    // too far off the size we need, so we dont assign a 1kb byte bucket for a
    // very small chunk
    byte* data_start = tag.data();
    for (mem_size i = 0; i < tag.bucket_count; i++) {
      BucketInfo* buck =
          reinterpret_cast<BucketInfo*>(page_end - (sizeof(BucketInfo) * (i + 1)));
      if (buck->IsFree() && buck->size() >= size) {
        float size_ratio = static_cast<float>(buck->size()) / static_cast<float>(size);
        if (size_ratio <= (1.0f + kSizeTolerance) &&
            size_ratio >= (1.0f - kSizeTolerance)) {
          buck->SetUsed();
          buck->SetSize(size);
          buck->SetUserSize(user_size);
          return buck;
        }
      }
    }

    // Check for gaps between existing buckets
    byte* last_data_end = data_start;
    for (mem_size i = 0; i < tag.bucket_count; i++) {
      BucketInfo* current_buck =
          reinterpret_cast<BucketInfo*>(page_end - (sizeof(BucketInfo) * (i + 1)));
      byte* current_data_start = data_start + current_buck->offset_;

      // Calculate gap between last bucket end and current bucket start
      mem_size gap_size = current_data_start - last_data_end;
      if (gap_size >= size) {
        // Found a suitable gap, create new bucket metadata
        BucketInfo* free_bucket =
            new (page_end - (sizeof(BucketInfo) * (node->value()->tag.bucket_count + 1)))
                BucketInfo(/*offset*/ last_data_end - data_start, user_size, size,
                           /*flags*/ BucketInfo::kUsed);
        node->value()->tag.bucket_count++;
        return free_bucket;
      }

      last_data_end = current_data_start + current_buck->size();
    }

    // No suitable gap found, allocate at the end
    byte* data_end =
        page_end - (sizeof(BucketInfo) * (node->value()->tag.bucket_count + 1));
    BucketInfo* last_buck = reinterpret_cast<BucketInfo*>(
        page_end - (sizeof(BucketInfo) * (node->value()->tag.bucket_count)));
    auto offset = last_buck->offset_ + last_buck->size();
    if (data_end - (data_start + offset) < size) {
      DEBUG_TRAP;  // not enough space
      return nullptr;
    }

    BucketInfo* free_bucket = new (data_end) BucketInfo(
        /*offset*/ offset, user_size, size, /*flags*/ BucketInfo::kUsed);
    node->value()->tag.bucket_count++;
    return free_bucket;
  }
  return nullptr;
}

BucketAllocator::BucketInfo* BucketAllocator::FindBucket(pointer_size address) {
  address -= sizeof(HeaderNode);

  for (auto* node = page_list_.head(); node != page_list_.end(); node = node->next()) {
    byte* page_start = reinterpret_cast<byte*>(node);
    byte* page_end = node->value()->tag.end();

    if (address < reinterpret_cast<pointer_size>(page_start) &&
        address > reinterpret_cast<pointer_size>(page_end)) {
      continue;
    }

    for (auto i = 0; i < node->value()->tag.bucket_count; i++) {
      BucketInfo* buck =
          reinterpret_cast<BucketInfo*>(page_end - (sizeof(BucketInfo) * (i + 1)));
      if (address >= (reinterpret_cast<pointer_size>(page_start) + buck->offset_) &&
          address < (reinterpret_cast<pointer_size>(page_start) +
                     (buck->offset_ + buck->size()))) {
        return buck;
      }
    }
  }
  return nullptr;
}

mem_size BucketAllocator::Free(void* pointer) {
  if (BucketInfo* b = FindBucket(reinterpret_cast<pointer_size>(pointer))) {
    const auto size = b->user_size();  // we do a little bit of trolling here
    b->SetFree();
    return size;
  }
  DCHECK(false, "BucketAllocator::Free(): Failed to release memory");
  return 0u;
}

mem_size BucketAllocator::QueryAllocationSize(void* block) {
  if (BucketInfo* b = FindBucket(reinterpret_cast<pointer_size>(block))) {
    return b->user_size();
  }
  return 0u;
}
}  // namespace base
