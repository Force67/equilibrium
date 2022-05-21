#pragma once

#include <base/memory/memory_literals.h>

namespace base::eq_allocation_constants {
using namespace base::memory_literals;

// handles smaller allocations < 1KiB. Uses a log2 based bucket sizing scheme.
// each bucket manages its own memory pages.
constexpr size_t kBucketThreshold = 1_kib;

constexpr size_t kPageThreshold = 64_kib;

// we use 64kib memory pages and align all pages to 1 mib boundaries.
// allocates full memory pages, for a large items, such as vertex buffers.
constexpr size_t kPageSize = 64_kib;
}  // namespace base::eq_allocation_constants