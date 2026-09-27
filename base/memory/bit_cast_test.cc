// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/arch.h>
#include <base/memory/bit_cast.h>

namespace {
TEST(BitCast, FloatRoundTrips) {
  EXPECT_EQ(base::BitCast<u32>(1.0f), 0x3f800000u);
  EXPECT_EQ(base::BitCast<f32>(0x40490fdbu), 3.14159274f);
  EXPECT_EQ(base::BitCast<f64>(base::BitCast<u64>(-2.5)), -2.5);
}

TEST(BitCast, IsConstexpr) {
  static_assert(base::BitCast<u32>(1.0f) == 0x3f800000u);
}
}  // namespace
