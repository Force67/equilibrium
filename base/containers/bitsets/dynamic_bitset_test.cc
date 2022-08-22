// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/containers/bitsets/dynamic_bitset.h>

#if 0
namespace {
using namespace base;

TEST(DynamicBitSet, Compare) {
	// this is leaking memory..
  base::DynamicBitSet a(9, 4);  // 1001
  base::DynamicBitSet b(3, 4);  // 0011
  ASSERT_FALSE(a == b);
  ASSERT_TRUE(a != b);
}

// LEAKING MEMORY!
TEST(DynamicBitSet, BitwiseAssignOP) {
  base::DynamicBitSet a(9, 4);  // 1001
  base::DynamicBitSet b(3, 4);  // 0011

  EXPECT_EQ(a.to_ullong(), 9);
  EXPECT_EQ(b.to_ullong(), 3);

  a ^= b;  // 1010
  EXPECT_EQ(a.to_ullong(), 10);

  // restore state
  a = 10;
  b = 3;

  a &= b;
  EXPECT_EQ(a.to_ullong(), 0b0010);

  // restore state
  a = 2;
  b = 3;

  a |= b;                       // OR
  EXPECT_EQ(a.to_ullong(), 3);  // 11
}

TEST(DynamicBitSet, SetTest) {
  base::DynamicBitSet a(9, 4);  // 1001

  a.Set(3, true);
  EXPECT_TRUE(a.Test(3));
}


TEST(DynamicBitSet, CountSetBits) {
  base::DynamicBitSet a(9, 4);  // 1001

  EXPECT_EQ(a.CountSetBits(), 2);
}

}  // namespace
#endif