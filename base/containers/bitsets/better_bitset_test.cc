// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/containers/bitsets/better_bitset.h>

#if 1
namespace {
using namespace base;

TEST(BetterBitSet, Compare) {
  base::BetterBitSet<4> a(9);  // 1001
  base::BetterBitSet<4> b(3);  // 0011
  ASSERT_FALSE(a == b);
  ASSERT_TRUE(a != b);
}

// https://www.geeksforgeeks.org/c-bitset-and-its-application/
TEST(BetterBitSet, BitwiseAssignOP) {
  base::BetterBitSet<4> a(9);  // 1001
  base::BetterBitSet<4> b(3);  // 0011

  EXPECT_EQ(a.to_number(), 9);
  EXPECT_EQ(b.to_number(), 3);

  a ^= b;  // 1010
  EXPECT_EQ(a.to_number(), 10);

  // restore state
  a = 10;
  b = 3;

  a &= b;
  EXPECT_EQ(a.to_number(), 0b0010);

  // restore state
  a = 2;
  b = 3;

  a |= b;                       // OR
  EXPECT_EQ(a.to_number(), 3);  // 11
}

TEST(BetterBitSet, BitwiseOP) {
  base::BetterBitSet<4> a(7);
  base::BetterBitSet<4> b(4);

  auto x = a & b;
  EXPECT_EQ(x.to_number(), 0b100);

  x = a | b;
  EXPECT_EQ(x.to_number(), 0b111);

  x = a ^ b;
  EXPECT_EQ(x.to_number(), 0b11);
}

TEST(BetterBitSet, SetTest) {
  base::BetterBitSet<4> a(9);  // 1001

  a.Set(3, true);
  EXPECT_TRUE(a.Test(3));
}

TEST(BetterBitSet, CountSetBits) {
  base::BetterBitSet<4> a(9);  // 1001
  EXPECT_EQ(a.CountSetBits(), 2);
}
}  // namespace
#endif