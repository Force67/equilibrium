// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#include <gtest/gtest.h>

#include <base/memory/allocation_size.h>
#include <base/numeric_limits.h>

namespace {
constexpr mem_size kMax = base::MinMax<mem_size>::max();

TEST(AllocationSize, ProductOfRepresentableCounts) {
  mem_size bytes = 0;
  EXPECT_TRUE(base::TryAllocationSize(0, 8, bytes));
  EXPECT_EQ(bytes, 0u);
  EXPECT_TRUE(base::TryAllocationSize(1000, 24, bytes));
  EXPECT_EQ(bytes, 24000u);
  // The largest count that still fits exactly.
  EXPECT_TRUE(base::TryAllocationSize(kMax / 8, 8, bytes));
  EXPECT_EQ(bytes, (kMax / 8) * 8);
}

TEST(AllocationSize, RejectsProductThatDoesNotFit) {
  mem_size bytes = 0xdeadbeef;
  EXPECT_FALSE(base::TryAllocationSize(kMax / 8 + 1, 8, bytes));
  EXPECT_FALSE(base::TryAllocationSize(kMax, 2, bytes));
  EXPECT_FALSE(base::TryAllocationSize(kMax, kMax, bytes));
  // Refused rather than wrapped: an unchecked multiply would report 0 bytes
  // here, and 0 is a size a container happily allocates.
  EXPECT_EQ(bytes, 0xdeadbeefu);
}

TEST(AllocationSize, ZeroElementSizeCannotOverflow) {
  mem_size bytes = 1;
  EXPECT_TRUE(base::TryAllocationSize(kMax, 0, bytes));
  EXPECT_EQ(bytes, 0u);
}

TEST(AllocationSize, RoundsUpToPowerOfTwo) {
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(0), 1u);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(1), 1u);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(2), 2u);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(3), 4u);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(17), 32u);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo(mem_size(1) << 62),
            mem_size(1) << 62);
  EXPECT_EQ(base::CheckedRoundUpToPowerOfTwo((mem_size(1) << 62) + 1),
            mem_size(1) << 63);
}

TEST(AllocationSize, SumsAndProductsOfSmallCounts) {
  EXPECT_EQ(base::CheckedCountSum(0, 0), 0u);
  EXPECT_EQ(base::CheckedCountSum(kMax - 1, 1), kMax);
  EXPECT_EQ(base::CheckedCountProduct(3, 7), 21u);
}

// The helpers terminate rather than return, so the arithmetic that would have
// wrapped can never reach an allocation. `volatile` keeps the compiler from
// folding these into a constant expression, where the abort would be a
// compile error instead of the runtime death being tested.
TEST(AllocationSizeDeathTest, OverflowingSizeTerminates) {
  volatile mem_size count = kMax / 8 + 1;
  EXPECT_DEATH({ (void)base::CheckedAllocationSize(count, 8); }, "");
}

TEST(AllocationSizeDeathTest, OverflowingSumTerminates) {
  volatile mem_size a = kMax;
  EXPECT_DEATH({ (void)base::CheckedCountSum(a, 1); }, "");
}

TEST(AllocationSizeDeathTest, UnreachablePowerOfTwoTerminates) {
  volatile mem_size value = (mem_size(1) << 63) + 1;
  EXPECT_DEATH({ (void)base::CheckedRoundUpToPowerOfTwo(value); }, "");
}
}  // namespace
