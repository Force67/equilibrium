// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// This defines the memory routing stategy used by eq_alloc
#pragma once

#include <base/compiler.h>
#include <base/numeric_limits.h>

#include <base/allocator/memory_context.h>
#include <base/allocator/eq_alloc/allocator.h>
#include <base/allocator/eq_alloc/page_table.h>
#include <base/allocator/eq_alloc/eq_allocation_constants.h>

namespace base {
struct EQMemoryRouter {
  enum AllocatorID : allocator_id {
    kBucketAllocator,
    kPageAllocator,
    kHeapAllocator,
    kCustomAllocators,
  };

  STRONG_INLINE mem_size nextPowerOfTwo(mem_size size) {
    if (size == 0)
      return 1;
    if ((size & (size - 1)) == 0)
      return size;  // Already a power of 2
    while ((size & (size - 1)) > 0) {
      size = size & (size - 1);
    }
    return size << 1;
  }

  STRONG_INLINE void* Allocate(const mem_size size) {
    // acquire atleast once, so it is always initialized, this is a rather big hack,
    // but for now i guess we need to.
    (void)page_table();

    const auto allocator_id = MemoryScope::allocator_handle();
    if (allocator_id != MemoryScope::NoOverride) {
      return allocators_[allocator_id]->Allocate(size, 1042);
    }

    if (size <= eq_allocation_constants::kBucketThreshold)
      return allocators_[AllocatorID::kBucketAllocator]->Allocate(size, 4);
    if (size <= eq_allocation_constants::kPageThreshold)
      return allocators_[AllocatorID::kPageAllocator]->Allocate(
          size, eq_allocation_constants::kPageThreshold);
    return allocators_[AllocatorID::kHeapAllocator]->Allocate(size,
                                                              nextPowerOfTwo(size));
  }

  STRONG_INLINE void* AllocateAligned(mem_size size, mem_size alignment) {
    // Adjust the size to include extra space for alignment correction and storing
    // the adjustment.
    mem_size expanded_size = size + alignment + sizeof(mem_size);

    // Allocate a new block with the expanded size.
    void* raw_block = Allocate(expanded_size);

    if (!raw_block) {
      // Allocation failed.
      return nullptr;
    }

    // Calculate the adjustment by masking off the lower bits of the address,
    // to make it a multiple of the alignment.
    mem_size adjustment =
        alignment - (reinterpret_cast<mem_size>(raw_block) & (alignment - 1));

    // Calculate the aligned address.
    byte* aligned_block = reinterpret_cast<byte*>(raw_block) + adjustment;

    // Store the adjustment just before the aligned block.
    reinterpret_cast<mem_size*>(aligned_block)[-1] = adjustment;

    return aligned_block;
  }

  STRONG_INLINE void* ReAllocate(void* former,
                                 const mem_size new_size,
                                 pointer_diff& diff_out) {
    auto& page_tab = *page_table();
    diff_out = new_size - block_size(page_tab, former);

    auto* allocator = FindOwningAllocator(former);
    DCHECK(allocator, "ReAllocate(): Orphaned memory?");

    return allocator->ReAllocate(former, new_size);
  }

  STRONG_INLINE void* ReAllocateAligned(void* former_block,
                                        mem_size former_size,
                                        mem_size new_size,
                                        mem_size alignment) {
    __debugbreak();
    return nullptr;
#if 0
    // Allocate a new block with the desired size and alignment.
    void* new_block = allocators_[AllocatorID::kHeapAllocator]->AllocateAligned(
        new_size, alignment);

    if (!new_block) {
      // Allocation failed.
      return nullptr;
    }

    // Copy the old data to the new block.
    mem_size copy_size = former_size < new_size ? former_size : new_size;
    memcpy(new_block, former_block, copy_size);

    // Free the old block.
    allocators_[AllocatorID::kHeapAllocator]->Free(former_block);

    return new_block;
#endif
  }

  STRONG_INLINE mem_size Free(void* block) {
    auto& page_tab = *page_table();
    const mem_size bytes_freed{block_size(page_tab, block)};

    auto* allocator = FindOwningAllocator(block);
    DCHECK(allocator, "Free(): Orphaned memory?");

    if (!allocator || !allocator->Free(block))
      return 0u;

    return bytes_freed;
  }

  STRONG_INLINE bool Deallocate(void* block, mem_size size, mem_size alignment) {
    (void)alignment;
    (void)size;
    Free(block);
    return true;
  }

 private:
  mem_size block_size(PageTable& tab, void* block) { return 0; }

  Allocator* FindOwningAllocator(void* block) {
    auto index = reinterpret_cast<uintptr_t>(block) >> 20;
    if (allocator_mapping_table_[index]) {
      return allocators_[allocator_mapping_table_[index]];
    }
    return nullptr;
  }

  void InitializeAllocators(PageTable&);

  PageTable* page_table();

 private:
  //~EQMemoryRouter() { page_table()->~PageTable(); }

  alignas(PageTable) byte page_table_data_[4+sizeof(PageTable)]{};
  // see
  // https://cdn.discordapp.com/attachments/818575873203503165/1005495331636662373/unknown.png
  // every index (number stored from 1 - 255 at given position) corresponds to a page
  // at offset n
  Allocator* allocators_[base::MinMax<allocator_id>::max()]{nullptr};
  allocator_id allocator_mapping_table_[1_tib >> 20]{0xFF};
};

}  // namespace base
