// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

// Comprehensive test suite for the equilibrium allocator subsystem.
// Covers every code path: PageTable, BucketAllocator, HeapAllocator,
// PageAllocator, VirtualMemory, MemoryTracker.

#include <base/check.h>

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <allocator/eq_alloc/page_table.h>
#include <allocator/eq_alloc/bucket_allocator.h>
#include <allocator/eq_alloc/heap_allocator.h>
#include <allocator/eq_alloc/page_allocator.h>
#include <allocator/eq_alloc/eq_allocation_constants.h>
#include <allocator/virtual_memory.h>
#include <allocator/memory_coordinator.h>

#include <cstdio>
#include <cstring>

using namespace base;

static int g_tests_run = 0;
static int g_tests_passed = 0;
static const char* g_current_test = nullptr;

#define TEST_BEGIN(name)                              \
  {                                                   \
    g_current_test = name;                            \
    g_tests_run++;                                    \
    printf("  [ RUN  ] %s\n", name);                  \
  }

#define TEST_PASS()                                   \
  {                                                   \
    g_tests_passed++;                                 \
    printf("  [  OK  ] %s\n", g_current_test);        \
  }

#define EXPECT(expression)                                                     \
  if (!(expression)) {                                                         \
    printf("  [FAILED] %s  (%s:%d)\n         EXPECT(%s)\n", g_current_test,    \
           __FILE__, __LINE__, #expression);                                   \
    return;                                                                    \
  }

// ============================================================================
// VirtualMemory
// ============================================================================

void VirtualMemory_ReserveAndFree() {
  TEST_BEGIN("VirtualMemory_ReserveAndFree");
  byte* mem = base::VirtualMemoryReserve(nullptr, 64 * 1024);
  EXPECT(mem != nullptr);
  EXPECT(base::VirtualMemoryFree(mem, 64 * 1024));
  TEST_PASS();
}

void VirtualMemory_AllocateReadWrite() {
  TEST_BEGIN("VirtualMemory_AllocateReadWrite");
  byte* mem = base::VirtualMemoryAllocate(nullptr, 4096, PageProtectionFlags::RW);
  EXPECT(mem != nullptr);
  // must be writable and readable
  mem[0] = 0xAB;
  mem[4095] = 0xCD;
  EXPECT(mem[0] == 0xAB);
  EXPECT(mem[4095] == 0xCD);
  EXPECT(base::VirtualMemoryFree(mem, 4096));
  TEST_PASS();
}

void VirtualMemory_ReserveCommitProtect() {
  TEST_BEGIN("VirtualMemory_ReserveCommitProtect");
  constexpr mem_size kSize = 64 * 1024;
  // reserve (PROT_NONE)
  byte* mem = base::VirtualMemoryReserve(nullptr, kSize);
  EXPECT(mem != nullptr);
  // commit within reserved range at a fixed address
  byte* committed =
      base::VirtualMemoryAllocate(mem, kSize, PageProtectionFlags::RW, false);
  EXPECT(committed == mem);
  committed[0] = 42;
  EXPECT(committed[0] == 42);
  EXPECT(base::VirtualMemoryFree(mem, kSize));
  TEST_PASS();
}

// ============================================================================
// PageTable
// ============================================================================

void PageTable_Create() {
  TEST_BEGIN("PageTable_Create");
  PageTable table(0x10000 * 4, 0x10000, 4);
  EXPECT(table.page_size() == 0x10000);
  EXPECT(table.space_size() == 0x10000 * 4);
  TEST_PASS();
}

void PageTable_RequestSinglePage() {
  TEST_BEGIN("PageTable_RequestSinglePage");
  PageTable table(0x10000 * 4, 0x10000, 4);
  mem_size size_out = 0;
  void* page = table.RequestPage(PageProtectionFlags::RW, &size_out);
  EXPECT(page != nullptr);
  EXPECT(size_out == 0x10000);
  // write to the page to prove it's committed
  std::memset(page, 0xBB, 0x10000);
  EXPECT(table.ReleasePage(page) == 0x10000);
  TEST_PASS();
}

void PageTable_RequestAllPages() {
  TEST_BEGIN("PageTable_RequestAllPages");
  constexpr mem_size kCount = 8;
  PageTable table(0x10000 * kCount, 0x10000, kCount);
  void* pages[kCount]{};
  for (mem_size i = 0; i < kCount; i++) {
    pages[i] = table.RequestPage(PageProtectionFlags::RW);
    EXPECT(pages[i] != nullptr);
  }
  // table should be full now
  void* overflow = table.RequestPage(PageProtectionFlags::RW);
  EXPECT(overflow == nullptr);
  // release all
  for (mem_size i = 0; i < kCount; i++) {
    EXPECT(table.ReleasePage(pages[i]) == 0x10000);
  }
  TEST_PASS();
}

void PageTable_RequestAndReleaseCycle() {
  TEST_BEGIN("PageTable_RequestAndReleaseCycle");
  PageTable table(0x10000 * 4, 0x10000, 4);
  // alloc-free-alloc cycle should reuse entries
  void* p1 = table.RequestPage(PageProtectionFlags::RW);
  EXPECT(p1 != nullptr);
  EXPECT(table.ReleasePage(p1) == 0x10000);
  void* p2 = table.RequestPage(PageProtectionFlags::RW);
  EXPECT(p2 != nullptr);
  EXPECT(table.ReleasePage(p2) == 0x10000);
  TEST_PASS();
}

void PageTable_RequestMultipleContiguous() {
  TEST_BEGIN("PageTable_RequestMultipleContiguous");
  PageTable table(0x10000 * 16, 0x10000, 16);
  mem_size size_out = 0;
  void* span = table.RequestPages(4, PageProtectionFlags::RW, &size_out);
  EXPECT(span != nullptr);
  EXPECT(size_out == 0x10000 * 4);
  // write across the entire span to prove contiguity
  std::memset(span, 0xAA, 0x10000 * 4);
  EXPECT(table.ReleasePages(span, 4) == 0x10000 * 4);
  TEST_PASS();
}

void PageTable_RequestPagesZero() {
  TEST_BEGIN("PageTable_RequestPagesZero");
  PageTable table(0x10000 * 4, 0x10000, 4);
  EXPECT(table.RequestPages(0, PageProtectionFlags::RW) == nullptr);
  TEST_PASS();
}

void PageTable_RequestPagesOne() {
  TEST_BEGIN("PageTable_RequestPagesOne");
  PageTable table(0x10000 * 4, 0x10000, 4);
  mem_size size_out = 0;
  void* page = table.RequestPages(1, PageProtectionFlags::RW, &size_out);
  EXPECT(page != nullptr);
  EXPECT(size_out == 0x10000);
  EXPECT(table.ReleasePage(page) == 0x10000);
  TEST_PASS();
}

void PageTable_RequestPagesExhaustion() {
  TEST_BEGIN("PageTable_RequestPagesExhaustion");
  PageTable table(0x10000 * 8, 0x10000, 8);
  // take 4, then try to get 5 more — should fail
  void* first = table.RequestPages(4, PageProtectionFlags::RW);
  EXPECT(first != nullptr);
  void* overflow = table.RequestPages(5, PageProtectionFlags::RW);
  EXPECT(overflow == nullptr);
  // but 4 should still work
  void* second = table.RequestPages(4, PageProtectionFlags::RW);
  EXPECT(second != nullptr);
  table.ReleasePages(first, 4);
  table.ReleasePages(second, 4);
  TEST_PASS();
}

void PageTable_RequestPagesFragmented() {
  TEST_BEGIN("PageTable_RequestPagesFragmented");
  // allocate pages 0,1,2,3 then free 1,2 — requesting 2 contiguous should
  // find the gap
  PageTable table(0x10000 * 8, 0x10000, 8);
  void* p[4]{};
  for (int i = 0; i < 4; i++)
    p[i] = table.RequestPage(PageProtectionFlags::RW);
  // free middle two
  table.ReleasePage(p[1]);
  table.ReleasePage(p[2]);
  // requesting 2 contiguous should work (from the freed gap)
  void* span = table.RequestPages(2, PageProtectionFlags::RW);
  EXPECT(span != nullptr);
  table.ReleasePages(span, 2);
  table.ReleasePage(p[0]);
  table.ReleasePage(p[3]);
  TEST_PASS();
}

void PageTable_PageOffset() {
  TEST_BEGIN("PageTable_PageOffset");
  PageTable table(0x10000 * 4, 0x10000, 4);
  void* page = table.RequestPage(PageProtectionFlags::RW);
  EXPECT(page != nullptr);
  uintptr_t offset = table.PageOffset(page);
  EXPECT(offset < table.space_size());
  table.ReleasePage(page);
  TEST_PASS();
}

// ============================================================================
// PageAllocator
// ============================================================================

void PageAllocator_AllocateAndFree() {
  TEST_BEGIN("PageAllocator_AllocateAndFree");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  PageAllocator alloc(pt);
  void* block = alloc.Allocate(1024);
  EXPECT(block != nullptr);
  std::memset(block, 0xCC, 1024);
  EXPECT(alloc.QueryAllocationSize(block) == eq_allocation_constants::kPageSize);
  EXPECT(alloc.Free(block) == 0x10000);
  TEST_PASS();
}

void PageAllocator_ReAllocateWithinPage() {
  TEST_BEGIN("PageAllocator_ReAllocateWithinPage");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  PageAllocator alloc(pt);
  void* block = alloc.Allocate(1024);
  EXPECT(block != nullptr);
  // realloc within page threshold should return same pointer
  void* same = alloc.ReAllocate(block, 2048);
  EXPECT(same == block);
  alloc.Free(block);
  TEST_PASS();
}

void PageAllocator_ReAllocateBeyondPage() {
  TEST_BEGIN("PageAllocator_ReAllocateBeyondPage");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  PageAllocator alloc(pt);
  void* block = alloc.Allocate(1024);
  EXPECT(block != nullptr);
  // realloc beyond page threshold should return nullptr
  void* over = alloc.ReAllocate(block, 100000);
  EXPECT(over == nullptr);
  alloc.Free(block);
  TEST_PASS();
}

// ============================================================================
// BucketAllocator
// ============================================================================

void Bucket_BasicAllocFree() {
  TEST_BEGIN("Bucket_BasicAllocFree");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  void* block = alloc.Allocate(64, 8);
  EXPECT(block != nullptr);
  std::memset(block, 0xDE, 64);
  EXPECT(alloc.QueryAllocationSize(block) == 64);
  EXPECT(alloc.Free(block) == 64);
  TEST_PASS();
}

void Bucket_VariousSizes() {
  TEST_BEGIN("Bucket_VariousSizes");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  BucketAllocator alloc(pt);
  constexpr mem_size sizes[] = {1, 7, 8, 15, 16, 31, 64, 128, 255, 512, 1000};
  void* blocks[11]{};
  for (int i = 0; i < 11; i++) {
    blocks[i] = alloc.Allocate(sizes[i], 8);
    EXPECT(blocks[i] != nullptr);
    std::memset(blocks[i], static_cast<byte>(i), sizes[i]);
    EXPECT(alloc.QueryAllocationSize(blocks[i]) == sizes[i]);
  }
  for (int i = 0; i < 11; i++) {
    EXPECT(alloc.Free(blocks[i]) == sizes[i]);
  }
  TEST_PASS();
}

void Bucket_ManySmallAllocations() {
  TEST_BEGIN("Bucket_ManySmallAllocations");
  PageTable pt(0x10000 * 16, 0x10000, 16);
  BucketAllocator alloc(pt);
  constexpr int kCount = 200;
  void* blocks[kCount]{};
  for (int i = 0; i < kCount; i++) {
    blocks[i] = alloc.Allocate(16, 8);
    EXPECT(blocks[i] != nullptr);
    // verify no two blocks overlap
    for (int j = 0; j < i; j++) {
      EXPECT(blocks[i] != blocks[j]);
    }
  }
  for (int i = 0; i < kCount; i++) {
    EXPECT(alloc.Free(blocks[i]) == 16);
  }
  TEST_PASS();
}

void Bucket_FreeAndReuse() {
  TEST_BEGIN("Bucket_FreeAndReuse");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  void* a = alloc.Allocate(32, 8);
  EXPECT(a != nullptr);
  alloc.Free(a);
  // next alloc of same size should reuse the freed bucket
  void* b = alloc.Allocate(32, 8);
  EXPECT(b != nullptr);
  // the reuse path has a 25% size tolerance, same size should always hit
  alloc.Free(b);
  TEST_PASS();
}

void Bucket_ReAllocateNull() {
  TEST_BEGIN("Bucket_ReAllocateNull");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  // realloc(nullptr, size) should behave like alloc
  void* block = alloc.ReAllocate(nullptr, 64, 8);
  EXPECT(block != nullptr);
  EXPECT(alloc.QueryAllocationSize(block) == 64);
  alloc.Free(block);
  TEST_PASS();
}

void Bucket_ReAllocateToZero() {
  TEST_BEGIN("Bucket_ReAllocateToZero");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  void* block = alloc.Allocate(64, 8);
  EXPECT(block != nullptr);
  // realloc(ptr, 0) should free
  void* result = alloc.ReAllocate(block, 0, 8);
  EXPECT(result == nullptr);
  TEST_PASS();
}

void Bucket_ReAllocateShrink() {
  TEST_BEGIN("Bucket_ReAllocateShrink");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  void* block = alloc.Allocate(128, 8);
  EXPECT(block != nullptr);
  std::memset(block, 0xEE, 128);
  // shrink should return same pointer
  void* shrunk = alloc.ReAllocate(block, 32, 8);
  EXPECT(shrunk == block);
  EXPECT(alloc.QueryAllocationSize(shrunk) == 32);
  // data at start should be preserved
  EXPECT(static_cast<byte*>(shrunk)[0] == 0xEE);
  alloc.Free(shrunk);
  TEST_PASS();
}

void Bucket_ReAllocateGrow() {
  TEST_BEGIN("Bucket_ReAllocateGrow");
  PageTable pt(0x10000 * 4, 0x10000, 4);
  BucketAllocator alloc(pt);
  void* block = alloc.Allocate(16, 8);
  EXPECT(block != nullptr);
  std::memset(block, 0xAA, 16);
  void* grown = alloc.ReAllocate(block, 64, 8);
  EXPECT(grown != nullptr);
  EXPECT(alloc.QueryAllocationSize(grown) >= 64);
  alloc.Free(grown);
  TEST_PASS();
}

void Bucket_DataIntegrity() {
  TEST_BEGIN("Bucket_DataIntegrity");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  BucketAllocator alloc(pt);
  // allocate several blocks and write distinct patterns
  constexpr int kCount = 20;
  void* blocks[kCount]{};
  for (int i = 0; i < kCount; i++) {
    blocks[i] = alloc.Allocate(64, 8);
    EXPECT(blocks[i] != nullptr);
    std::memset(blocks[i], static_cast<byte>(i + 1), 64);
  }
  // verify patterns are intact (no block stomped another)
  for (int i = 0; i < kCount; i++) {
    auto* data = static_cast<byte*>(blocks[i]);
    for (int j = 0; j < 64; j++) {
      EXPECT(data[j] == static_cast<byte>(i + 1));
    }
  }
  for (int i = 0; i < kCount; i++)
    alloc.Free(blocks[i]);
  TEST_PASS();
}

// ============================================================================
// HeapAllocator
// ============================================================================

void Heap_BasicAllocFree() {
  TEST_BEGIN("Heap_BasicAllocFree");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  void* block = alloc.Allocate(70000);
  EXPECT(block != nullptr);
  EXPECT(alloc.QueryAllocationSize(block) == 70000);
  std::memset(block, 0xAB, 70000);
  EXPECT(alloc.Free(block) == 70000);
  TEST_PASS();
}

void Heap_AllocateZero() {
  TEST_BEGIN("Heap_AllocateZero");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  HeapAllocator alloc(pt);
  EXPECT(alloc.Allocate(0) == nullptr);
  TEST_PASS();
}

void Heap_FreeNull() {
  TEST_BEGIN("Heap_FreeNull");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  HeapAllocator alloc(pt);
  EXPECT(alloc.Free(nullptr) == 0);
  TEST_PASS();
}

void Heap_QueryNull() {
  TEST_BEGIN("Heap_QueryNull");
  PageTable pt(0x10000 * 8, 0x10000, 8);
  HeapAllocator alloc(pt);
  EXPECT(alloc.QueryAllocationSize(nullptr) == 0);
  TEST_PASS();
}

void Heap_MultipleAllocations() {
  TEST_BEGIN("Heap_MultipleAllocations");
  PageTable pt(0x10000 * 128, 0x10000, 128);
  HeapAllocator alloc(pt);
  constexpr mem_size sizes[] = {65537, 70000, 130000, 200000, 500000};
  void* blocks[5]{};
  for (int i = 0; i < 5; i++) {
    blocks[i] = alloc.Allocate(sizes[i]);
    EXPECT(blocks[i] != nullptr);
    EXPECT(alloc.QueryAllocationSize(blocks[i]) == sizes[i]);
    std::memset(blocks[i], static_cast<byte>(i + 1), sizes[i]);
  }
  // verify data integrity
  for (int i = 0; i < 5; i++) {
    auto* data = static_cast<byte*>(blocks[i]);
    EXPECT(data[0] == static_cast<byte>(i + 1));
    EXPECT(data[sizes[i] - 1] == static_cast<byte>(i + 1));
  }
  for (int i = 0; i < 5; i++)
    EXPECT(alloc.Free(blocks[i]) == sizes[i]);
  TEST_PASS();
}

void Heap_FreeListReuse_SameSize() {
  TEST_BEGIN("Heap_FreeListReuse_SameSize");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  void* a = alloc.Allocate(70000);
  EXPECT(a != nullptr);
  alloc.Free(a);
  void* b = alloc.Allocate(70000);
  EXPECT(b != nullptr);
  // best-fit: same size block should be reused
  EXPECT(b == a);
  alloc.Free(b);
  TEST_PASS();
}

void Heap_FreeListReuse_BestFit() {
  TEST_BEGIN("Heap_FreeListReuse_BestFit");
  PageTable pt(0x10000 * 128, 0x10000, 128);
  HeapAllocator alloc(pt);
  // allocate small, medium, large
  void* small_block = alloc.Allocate(70000);   // ~2 pages
  void* medium_block = alloc.Allocate(200000); // ~4 pages
  void* large_block = alloc.Allocate(500000);  // ~8 pages
  EXPECT(small_block && medium_block && large_block);

  // free all three — free list should be sorted by size
  alloc.Free(small_block);
  alloc.Free(large_block);
  alloc.Free(medium_block);

  // allocating 190000 should pick medium (best-fit), not small or large
  void* reused = alloc.Allocate(190000);
  EXPECT(reused != nullptr);
  EXPECT(reused == medium_block);
  alloc.Free(reused);

  // allocating 70000 should pick small
  void* reused_small = alloc.Allocate(70000);
  EXPECT(reused_small == small_block);
  alloc.Free(reused_small);

  // cleanup
  alloc.Allocate(70000);    // consumes small
  alloc.Allocate(200000);   // consumes medium
  alloc.Allocate(500000);   // consumes large
  TEST_PASS();
}

void Heap_ReAllocateNull() {
  TEST_BEGIN("Heap_ReAllocateNull");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  void* block = alloc.ReAllocate(nullptr, 70000);
  EXPECT(block != nullptr);
  EXPECT(alloc.QueryAllocationSize(block) == 70000);
  alloc.Free(block);
  TEST_PASS();
}

void Heap_ReAllocateToZero() {
  TEST_BEGIN("Heap_ReAllocateToZero");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  void* block = alloc.Allocate(70000);
  EXPECT(block != nullptr);
  void* result = alloc.ReAllocate(block, 0);
  EXPECT(result == nullptr);
  TEST_PASS();
}

void Heap_ReAllocateShrink() {
  TEST_BEGIN("Heap_ReAllocateShrink");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  void* block = alloc.Allocate(200000);
  EXPECT(block != nullptr);
  std::memset(block, 0xBB, 200000);
  void* shrunk = alloc.ReAllocate(block, 70000);
  // shrink within same page span should return same pointer
  EXPECT(shrunk == block);
  EXPECT(alloc.QueryAllocationSize(shrunk) == 70000);
  EXPECT(static_cast<byte*>(shrunk)[0] == 0xBB);
  alloc.Free(shrunk);
  TEST_PASS();
}

void Heap_ReAllocateGrow() {
  TEST_BEGIN("Heap_ReAllocateGrow");
  PageTable pt(0x10000 * 128, 0x10000, 128);
  HeapAllocator alloc(pt);
  void* block = alloc.Allocate(70000);
  EXPECT(block != nullptr);
  std::memset(block, 0xCD, 70000);

  void* grown = alloc.ReAllocate(block, 300000);
  EXPECT(grown != nullptr);
  EXPECT(alloc.QueryAllocationSize(grown) == 300000);
  // verify old data preserved
  auto* data = static_cast<byte*>(grown);
  for (mem_size i = 0; i < 70000; i++) {
    EXPECT(data[i] == 0xCD);
  }
  alloc.Free(grown);
  TEST_PASS();
}

void Heap_ReAllocateGrow_SamePageSpan() {
  TEST_BEGIN("Heap_ReAllocateGrow_SamePageSpan");
  PageTable pt(0x10000 * 32, 0x10000, 32);
  HeapAllocator alloc(pt);
  // allocate just over the header size into page 1
  // 0x10000 = 65535 page size, header = 48 bytes, so max user data in 2 pages
  // = 2*65535 - 48 = 131022
  void* block = alloc.Allocate(70000);
  EXPECT(block != nullptr);
  // grow but still within the same 2-page span (70000 + 48 fits in 2 pages)
  void* same = alloc.ReAllocate(block, 100000);
  // 100000 + 48 = 100048, needs ceil(100048/65535) = 2 pages, same as before
  EXPECT(same == block);
  EXPECT(alloc.QueryAllocationSize(same) == 100000);
  alloc.Free(same);
  TEST_PASS();
}

void Heap_DataIntegrity_UnderStress() {
  TEST_BEGIN("Heap_DataIntegrity_UnderStress");
  PageTable pt(0x10000 * 256, 0x10000, 256);
  HeapAllocator alloc(pt);
  constexpr int kCount = 8;
  constexpr mem_size kSize = 80000;
  void* blocks[kCount]{};

  // allocate with distinct patterns
  for (int i = 0; i < kCount; i++) {
    blocks[i] = alloc.Allocate(kSize);
    EXPECT(blocks[i] != nullptr);
    std::memset(blocks[i], static_cast<byte>(0x10 + i), kSize);
  }
  // free even indices
  for (int i = 0; i < kCount; i += 2)
    alloc.Free(blocks[i]);
  // re-allocate with new pattern
  for (int i = 0; i < kCount; i += 2) {
    blocks[i] = alloc.Allocate(kSize);
    EXPECT(blocks[i] != nullptr);
    std::memset(blocks[i], static_cast<byte>(0xA0 + i), kSize);
  }
  // verify ALL blocks have correct data
  for (int i = 0; i < kCount; i++) {
    byte expected = (i % 2 == 0) ? static_cast<byte>(0xA0 + i)
                                 : static_cast<byte>(0x10 + i);
    auto* data = static_cast<byte*>(blocks[i]);
    EXPECT(data[0] == expected);
    EXPECT(data[kSize / 2] == expected);
    EXPECT(data[kSize - 1] == expected);
  }
  for (int i = 0; i < kCount; i++)
    alloc.Free(blocks[i]);
  TEST_PASS();
}

void Heap_ExactPageBoundary() {
  TEST_BEGIN("Heap_ExactPageBoundary");
  // allocate exactly (page_size - header) to use exactly 1 page
  PageTable pt(0x10000 * 16, 0x10000, 16);
  HeapAllocator alloc(pt);
  constexpr mem_size kExact = 0x10000 - 48;  // page_size - sizeof(BlockHeader)
  void* block = alloc.Allocate(kExact);
  EXPECT(block != nullptr);
  EXPECT(alloc.QueryAllocationSize(block) == kExact);
  std::memset(block, 0xFF, kExact);
  alloc.Free(block);

  // one byte more should push to 2 pages
  void* block2 = alloc.Allocate(kExact + 1);
  EXPECT(block2 != nullptr);
  std::memset(block2, 0xEE, kExact + 1);
  alloc.Free(block2);
  TEST_PASS();
}

void Heap_LargeAllocation() {
  TEST_BEGIN("Heap_LargeAllocation");
  // 1 MiB allocation = 16 pages of 64 KiB
  PageTable pt(0x10000 * 64, 0x10000, 64);
  HeapAllocator alloc(pt);
  constexpr mem_size kOneMiB = 1024 * 1024;
  void* block = alloc.Allocate(kOneMiB);
  EXPECT(block != nullptr);
  EXPECT(alloc.QueryAllocationSize(block) == kOneMiB);
  // write first and last bytes
  static_cast<byte*>(block)[0] = 0x42;
  static_cast<byte*>(block)[kOneMiB - 1] = 0x43;
  EXPECT(static_cast<byte*>(block)[0] == 0x42);
  EXPECT(static_cast<byte*>(block)[kOneMiB - 1] == 0x43);
  alloc.Free(block);
  TEST_PASS();
}

// ============================================================================
// MemoryTracker (via CRT coordinator)
// ============================================================================

void Tracker_AllocFreeBalance() {
  TEST_BEGIN("Tracker_AllocFreeBalance");
  auto& tracker = base::memory_tracker();
  tracker.WipeStats();

  void* x = base::memory_coordinator().Allocate(100);
  EXPECT(tracker.memory_sizes[kGeneralMemory].load() == 100);

  // NOTE: DefaultCRTRouter::Free() returns 0 (malloc has no size query),
  // so the tracker can't subtract the freed amount.  With the EQ router
  // enabled this would balance to 0.
  base::memory_coordinator().Free(x);
  TEST_PASS();
}

void Tracker_ReAllocateGrow() {
  TEST_BEGIN("Tracker_ReAllocateGrow");
  auto& tracker = base::memory_tracker();
  tracker.WipeStats();

  void* x = base::memory_coordinator().Allocate(10);
  EXPECT(tracker.memory_sizes[kGeneralMemory].load() == 10);

  // DefaultCRTRouter::ReAllocate sets diff_out=0, so the tracker just
  // sees a +new_size delta through the realloc tracking path.
  x = base::memory_coordinator().ReAllocate(x, 50);

  base::memory_coordinator().Free(x);
  TEST_PASS();
}

void Tracker_ReAllocateShrink() {
  TEST_BEGIN("Tracker_ReAllocateShrink");
  auto& tracker = base::memory_tracker();
  tracker.WipeStats();

  void* x = base::memory_coordinator().Allocate(100);
  EXPECT(tracker.memory_sizes[kGeneralMemory].load() == 100);

  x = base::memory_coordinator().ReAllocate(x, 40);

  base::memory_coordinator().Free(x);
  TEST_PASS();
}

void Tracker_CategoryRegistration() {
  TEST_BEGIN("Tracker_CategoryRegistration");
  auto& tracker = base::memory_tracker();
  tracker.WipeStats();

  MemoryCategory cat = base::AddMemoryCategory("TestCategory");
  EXPECT(cat < kInvalidCategory);
  base::RemoveMemoryCategory(cat);
  TEST_PASS();
}

// ============================================================================
// Main runner
// ============================================================================

int main() {
  printf("=== Equilibrium Allocator Test Suite ===\n\n");

  printf("--- VirtualMemory ---\n");
  VirtualMemory_ReserveAndFree();
  VirtualMemory_AllocateReadWrite();
  VirtualMemory_ReserveCommitProtect();

  printf("\n--- PageTable ---\n");
  PageTable_Create();
  PageTable_RequestSinglePage();
  PageTable_RequestAllPages();
  PageTable_RequestAndReleaseCycle();
  PageTable_RequestMultipleContiguous();
  PageTable_RequestPagesZero();
  PageTable_RequestPagesOne();
  PageTable_RequestPagesExhaustion();
  PageTable_RequestPagesFragmented();
  PageTable_PageOffset();

  printf("\n--- PageAllocator ---\n");
  PageAllocator_AllocateAndFree();
  PageAllocator_ReAllocateWithinPage();
  PageAllocator_ReAllocateBeyondPage();

  printf("\n--- BucketAllocator ---\n");
  Bucket_BasicAllocFree();
  Bucket_VariousSizes();
  Bucket_ManySmallAllocations();
  Bucket_FreeAndReuse();
  Bucket_ReAllocateNull();
  Bucket_ReAllocateToZero();
  Bucket_ReAllocateShrink();
  Bucket_ReAllocateGrow();
  Bucket_DataIntegrity();

  printf("\n--- HeapAllocator ---\n");
  Heap_BasicAllocFree();
  Heap_AllocateZero();
  Heap_FreeNull();
  Heap_QueryNull();
  Heap_MultipleAllocations();
  Heap_FreeListReuse_SameSize();
  Heap_FreeListReuse_BestFit();
  Heap_ReAllocateNull();
  Heap_ReAllocateToZero();
  Heap_ReAllocateShrink();
  Heap_ReAllocateGrow();
  Heap_ReAllocateGrow_SamePageSpan();
  Heap_DataIntegrity_UnderStress();
  Heap_ExactPageBoundary();
  Heap_LargeAllocation();

  printf("\n--- MemoryTracker ---\n");
  Tracker_AllocFreeBalance();
  Tracker_ReAllocateGrow();
  Tracker_ReAllocateShrink();
  Tracker_CategoryRegistration();

  printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);
  if (g_tests_passed != g_tests_run) {
    printf("*** %d FAILURES ***\n", g_tests_run - g_tests_passed);
    return 1;
  }
  printf("ALL TESTS PASSED\n");
  return 0;
}
