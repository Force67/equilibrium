// eq_alloc vs mimalloc benchmark
// Compares allocation throughput across the three allocator tiers:
//   Bucket (<= 1 KiB), Page (1-64 KiB), Heap (> 64 KiB)
// Plus mixed workloads and contention patterns.

#include <base/check.h>
#include <base/containers/vector.h>
#include <base/time/time.h>

#define BASE_MAY_USE_MEMORY_COORDINATOR
#include <allocator/eq_alloc/page_table.h>
#include <allocator/eq_alloc/bucket_allocator.h>
#include <allocator/eq_alloc/heap_allocator.h>
#include <allocator/eq_alloc/page_allocator.h>
#include <allocator/eq_alloc/eq_allocation_constants.h>
#include <allocator/virtual_memory.h>

#include <mimalloc.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>

using namespace base;

struct BenchResult {
  const char* name;
  double eq_ns;
  double mi_ns;
};

static double NsPerOp(i64 dur_ns, int ops) {
  return static_cast<double>(dur_ns) / ops;
}

static void PrintResult(const BenchResult& r) {
  double ratio = r.eq_ns / r.mi_ns;
  const char* verdict = ratio < 1.0 ? "eq FASTER" : ratio > 1.0 ? "mi FASTER" : "TIED";
  printf("  %-40s  eq: %8.1f ns/op   mi: %8.1f ns/op   ratio: %.2fx  [%s]\n",
         r.name, r.eq_ns, r.mi_ns, ratio, verdict);
}

// prevent the compiler from optimizing away our allocations
static volatile void* g_sink;
#define DONT_OPTIMIZE(p) g_sink = (p)

// 1. Small allocations (bucket tier: <= 1 KiB)
BenchResult BenchSmallAlloc(int n) {
  // -- eq_alloc (BucketAllocator) --
  PageTable pt(0x10000ULL * 512, 0x10000, 512);
  BucketAllocator bucket(pt);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = bucket.Allocate(64, 8);
    DONT_OPTIMIZE(p);
    bucket.Free(p);
  }
  auto eq_dur = base::TickClock::NowNs() - t0;

  // -- mimalloc --
  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = mi_malloc(64);
    DONT_OPTIMIZE(p);
    mi_free(p);
  }
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Small (64 B) alloc+free", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 2. Medium allocations (page tier: 1-64 KiB)
BenchResult BenchMediumAlloc(int n) {
  PageTable pt(0x10000ULL * 2048, 0x10000, 2048);
  PageAllocator page(pt);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = page.Allocate(4096);
    DONT_OPTIMIZE(p);
    page.Free(p);
  }
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = mi_malloc(4096);
    DONT_OPTIMIZE(p);
    mi_free(p);
  }
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Medium (4 KiB) alloc+free", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 3. Large allocations (heap tier: > 64 KiB)
BenchResult BenchLargeAlloc(int n) {
  PageTable pt(0x10000ULL * 4096, 0x10000, 4096);
  HeapAllocator heap(pt);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = heap.Allocate(128 * 1024);
    DONT_OPTIMIZE(p);
    heap.Free(p);
  }
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = mi_malloc(128 * 1024);
    DONT_OPTIMIZE(p);
    mi_free(p);
  }
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Large (128 KiB) alloc+free", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 4. Bulk small — allocate N then free N (tests sustained throughput, not
//    just the hot free-list path)
BenchResult BenchBulkSmall(int n) {
  PageTable pt(0x10000ULL * 4096, 0x10000, 4096);
  BucketAllocator bucket(pt);
  base::Vector<void*> ptrs(n);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = bucket.Allocate(48, 8);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = 0; i < n; i++)
    bucket.Free(ptrs[i]);
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = mi_malloc(48);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = 0; i < n; i++)
    mi_free(ptrs[i]);
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Bulk small (48 B) x N", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 5. Bulk large
BenchResult BenchBulkLarge(int n) {
  PageTable pt(0x10000ULL * 8192, 0x10000, 8192);
  HeapAllocator heap(pt);
  base::Vector<void*> ptrs(n);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = heap.Allocate(256 * 1024);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = 0; i < n; i++)
    heap.Free(ptrs[i]);
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = mi_malloc(256 * 1024);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = 0; i < n; i++)
    mi_free(ptrs[i]);
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Bulk large (256 KiB) x N", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 6. Mixed sizes — simulates a realistic game-frame workload
BenchResult BenchMixedSizes(int n) {
  constexpr mem_size kSizes[] = {16, 32, 64, 128, 256, 512, 1024};
  constexpr int kNumSizes = sizeof(kSizes) / sizeof(kSizes[0]);

  PageTable pt(0x10000ULL * 4096, 0x10000, 4096);
  BucketAllocator bucket(pt);
  base::Vector<void*> ptrs(n);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = bucket.Allocate(kSizes[i % kNumSizes], 8);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = n - 1; i >= 0; i--)
    bucket.Free(ptrs[i]);
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    ptrs[i] = mi_malloc(kSizes[i % kNumSizes]);
    DONT_OPTIMIZE(ptrs[i]);
  }
  for (int i = n - 1; i >= 0; i--)
    mi_free(ptrs[i]);
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Mixed sizes (16-1024 B)", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 7. Churn — interleaved alloc/free simulating a frame allocator pattern
BenchResult BenchChurn(int n) {
  PageTable pt(0x10000ULL * 512, 0x10000, 512);
  BucketAllocator bucket(pt);

  constexpr int kWindow = 32;
  void* window[kWindow]{};

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    int slot = i % kWindow;
    if (window[slot])
      bucket.Free(window[slot]);
    window[slot] = bucket.Allocate(96, 8);
    DONT_OPTIMIZE(window[slot]);
  }
  for (auto& p : window)
    if (p) bucket.Free(p);
  auto eq_dur = base::TickClock::NowNs() - t0;

  void* mi_window[kWindow]{};
  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    int slot = i % kWindow;
    if (mi_window[slot])
      mi_free(mi_window[slot]);
    mi_window[slot] = mi_malloc(96);
    DONT_OPTIMIZE(mi_window[slot]);
  }
  for (auto& p : mi_window)
    if (p) mi_free(p);
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Churn (96 B, 32-slot window)", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

// 8. Realloc pattern
BenchResult BenchRealloc(int n) {
  PageTable pt(0x10000ULL * 4096, 0x10000, 4096);
  HeapAllocator heap(pt);

  auto t0 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = heap.Allocate(70000);
    p = heap.ReAllocate(p, 140000);
    DONT_OPTIMIZE(p);
    heap.Free(p);
  }
  auto eq_dur = base::TickClock::NowNs() - t0;

  auto t1 = base::TickClock::NowNs();
  for (int i = 0; i < n; i++) {
    void* p = mi_malloc(70000);
    p = mi_realloc(p, 140000);
    DONT_OPTIMIZE(p);
    mi_free(p);
  }
  auto mi_dur = base::TickClock::NowNs() - t1;

  return {"Realloc (70K -> 140K)", NsPerOp(eq_dur, n), NsPerOp(mi_dur, n)};
}

int main() {
  printf("=== eq_alloc vs mimalloc benchmark ===\n");
  printf("Warming up...\n\n");

  // warmup to stabilize caches/TLB
  for (int i = 0; i < 10000; i++) {
    void* p = mi_malloc(64);
    mi_free(p);
  }

  constexpr int kSmall = 500000;
  constexpr int kMedium = 100000;
  constexpr int kLarge = 10000;
  constexpr int kBulkSmall = 50000;
  constexpr int kBulkLarge = 500;

  BenchResult results[] = {
      BenchSmallAlloc(kSmall),
      BenchMediumAlloc(kMedium),
      BenchLargeAlloc(kLarge),
      BenchBulkSmall(kBulkSmall),
      BenchBulkLarge(kBulkLarge),
      BenchMixedSizes(20000),
      BenchChurn(kSmall),
      BenchRealloc(kLarge),
  };

  printf("%-42s  %-22s  %-22s  %-10s\n",
         "Benchmark", "eq_alloc", "mimalloc", "ratio");
  printf("----------------------------------------------"
         "----------------------------------------------"
         "----------\n");
  for (auto& r : results)
    PrintResult(r);

  printf("\nratio < 1.0 = eq_alloc wins, > 1.0 = mimalloc wins\n");
  return 0;
}
