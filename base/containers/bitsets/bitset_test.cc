// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/containers/bitsets/bitset.h>

namespace {
using namespace base;

TEST(BitSetTest, Compare) {
  base::BitSet<4> a(9); // 1001
  base::BitSet<4> b(3); // 0011
  ASSERT_FALSE(a == b);
  ASSERT_TRUE(a != b);
}

// https://www.geeksforgeeks.org/c-bitset-and-its-application/
TEST(BitSetTest, BitwiseAssignOP) {
  base::BitSet<4> a(9);  // 1001
  base::BitSet<4> b(3);  // 0011

  a ^= b;
  EXPECT_EQ(a.to_ulong(), 10);

  a &= b;
  EXPECT_EQ(a.to_ulong(), 2);

  a |= b;
  EXPECT_EQ(a.to_ulong(), 3);
}

TEST(BitSetTest, BitwiseOP) {
  base::BitSet<4> a(9);  // 1001
  base::BitSet<4> b(3);  // 0011

  auto x = a & b;
  EXPECT_EQ(x.to_ulong(), 1);

  x = a | b;
  EXPECT_EQ(x.to_ulong(), 11);

  x = a ^ b;
  EXPECT_EQ(x.to_ulong(), 10);
}

TEST(BitSetTest, Shift) {
  base::BitSet<4> a(9);  // 1001

  a <<= 2;
  EXPECT_EQ(a.to_ullong(), 36);

  a >>= 1;
  EXPECT_EQ(a.to_ullong(), 18);
}

TEST(BitSetTest, SetTest) {
  base::BitSet<4> a(9);  // 1001

  a.Set(3, true);
  EXPECT_TRUE(a.Test(3));
}

TEST(BitSetTest, CountSetBits) {
  base::BitSet<4> a(9);  // 1001

  EXPECT_EQ(a.CountSetBits(), 2);
}
}  // namespace
