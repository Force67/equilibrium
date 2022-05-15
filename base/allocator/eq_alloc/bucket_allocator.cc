// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <new>
#include <base/check.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/bucket_allocator.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

namespace base {
BucketAllocator::BucketAllocator() {}

// TODO: worry about thread safety
// TODO:   // log2 based alignment
void* BucketAllocator::Allocate(PageTable& pages, size_t size, size_t alignment) {
  if (size > eq_allocation_constants::kBucketThreshold)
    return nullptr;

  size += sizeof(Bucket);

  if (void* block = AcquireMemory(size))
    return block;

  // failed to find a bucket in all current pages, acquire a new one
  if (!TryAcquireNewPage(pages))
    return nullptr;

  // try again (not in a loop, that could have unforseen consequences)
  // TODO: maybe give a hint about the page to start using?
  if (void* block = AcquireMemory(size))
    return block;

  return nullptr;
}

void* BucketAllocator::AcquireMemory(mem_size size) {
  byte* page_head = nullptr;
  if (Bucket* bucket = FindFreeBucket(size, page_head)) {
    // user memory begins after page tag
    return reinterpret_cast<void*>(page_head + 
        sizeof(PageHeader) + bucket->offset_);
  }
  return nullptr;
}

bool BucketAllocator::TryAcquireNewPage(PageTable& table) {
  size_t page_size = 0;
  byte* page = static_cast<byte*>(table.RequestPage(page_size));
  if (!page || page_size == 0) {
    DCHECK(false, "page or page size invalid");
    return false;
  }
  // store the entire node in the page itself
  auto* entry = new (page) HeaderNode();
  entry->value()->header.page_size = page_size;

  // value is returning garbrage
  byte* page_end = page + page_size;
  // set the initial bucket
  entry->value()->header.bucket_table =
      reinterpret_cast<Bucket*>(page_end - sizeof(Bucket));
  page_list_.Append(entry);
  return true;
}

void BucketAllocator::TakeMemoryChunk(Bucket& bucket,
                                      uint8_t* start_hint,
                                      mem_size req_size) {
  bucket.offset_ = 0;
  bucket.flags_ = 0;
  bucket.size_ = static_cast<u16>(req_size);
}

BucketAllocator::Bucket* BucketAllocator::FindFreeBucket(mem_size requested_size,
                                                         byte*& page_start) {
  for (auto* node = page_list_.head(); node != page_list_.end();
       node = node->next()) {
    PageHeader& header = node->value()->header;
    page_start = reinterpret_cast<byte*>(node);

    // looks like a fresh page
    if (header.bucket_count == 0 && header.bucket_table) {
      TakeMemoryChunk(*header.bucket_table, page_start, requested_size);
      header.bucket_count++;
      return header.bucket_table;
    }

    byte* page_end = reinterpret_cast<byte*>(node) + header.page_size;

    // attempt reclaiming unused buckets (and their memory, which is orphaned)
    for (auto i = 0; i < header.bucket_count; i++) {
      Bucket* buck = reinterpret_cast<Bucket*>(page_end - (sizeof(Bucket) * i));
      // reclaim previously used bucket
      //
      // TODO: should make a resonable choice, based on alignment, if we can reclaim
      // the block rather than relying on the same size...
      if (buck->flags_ == 1 && buck->size_ >= requested_size) {
        TakeMemoryChunk(*buck, page_start + sizeof(PageHeader) + buck->offset_,
                        requested_size);
        header.bucket_count++;
        return buck;
      }
    }
    // new bucket
    // todo: validate if buckets are owned by any other memory high prio!!!
    Bucket* free_bucket = reinterpret_cast<Bucket*>(
        page_end - (sizeof(Bucket) * (header.bucket_count + 1)));
    free_bucket->flags_ = 0;
    free_bucket->size_ = static_cast<u16>(requested_size);
    // free_bucket->offset_ =

    return free_bucket;
  }
  return nullptr;
}

void BucketAllocator::Free(void* pointer) {
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
}
}  // namespace base