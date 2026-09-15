// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/arch.h>
#include <base/compiler.h>
#include <base/containers/builtins_bit.h>

namespace {

TEST(BuiltinsBit, CountLeftZeroCountsFromTheTypeWidth) {
  // The width is per-type, not per-value: the same 1 leaves a different
  // number of leading zeros in each.
  EXPECT_EQ(base::CountLeftZero(static_cast<u8>(1)), 7);
  EXPECT_EQ(base::CountLeftZero(static_cast<u16>(1)), 15);
  EXPECT_EQ(base::CountLeftZero(static_cast<u32>(1)), 31);
  EXPECT_EQ(base::CountLeftZero(static_cast<u64>(1)), 63);
}

TEST(BuiltinsBit, CountLeftZeroOnZeroIsTheFullWidth) {
  // The underlying builtins are undefined at zero, so this is the case the
  // wrapper exists to pin.
  EXPECT_EQ(base::CountLeftZero(static_cast<u8>(0)), 8);
  EXPECT_EQ(base::CountLeftZero(static_cast<u16>(0)), 16);
  EXPECT_EQ(base::CountLeftZero(static_cast<u32>(0)), 32);
  EXPECT_EQ(base::CountLeftZero(static_cast<u64>(0)), 64);
}

TEST(BuiltinsBit, CountLeftZeroAtEachBitPosition) {
  for (int bit = 0; bit < 32; bit++) {
    const u32 value = u32{1} << bit;
    EXPECT_EQ(base::CountLeftZero(value), 31 - bit) << "bit " << bit;
    // A lower bit set as well must not move the answer.
    EXPECT_EQ(base::CountLeftZero(static_cast<u32>(value | 1u)), 31 - bit);
  }
  for (int bit = 0; bit < 64; bit++) {
    const u64 value = u64{1} << bit;
    EXPECT_EQ(base::CountLeftZero(value), 63 - bit) << "bit " << bit;
  }
}

TEST(BuiltinsBit, CountLeftZeroOnAllOnesIsZero) {
  EXPECT_EQ(base::CountLeftZero(static_cast<u8>(0xFF)), 0);
  EXPECT_EQ(base::CountLeftZero(static_cast<u16>(0xFFFF)), 0);
  EXPECT_EQ(base::CountLeftZero(static_cast<u32>(0xFFFFFFFFu)), 0);
  EXPECT_EQ(base::CountLeftZero(static_cast<u64>(~u64{0})), 0);
}

TEST(BuiltinsBit, CountRightZeroAtEachBitPosition) {
  for (int bit = 0; bit < 32; bit++) {
    const u32 value = u32{1} << bit;
    EXPECT_EQ(base::CountRightZero(value), bit) << "bit " << bit;
    // A higher bit set as well must not move the answer.
    EXPECT_EQ(base::CountRightZero(static_cast<u32>(value | (1u << 31))), bit);
  }
  for (int bit = 0; bit < 64; bit++) {
    const u64 value = u64{1} << bit;
    EXPECT_EQ(base::CountRightZero(value), bit) << "bit " << bit;
  }
}

TEST(BuiltinsBit, CountRightZeroOnZeroIsTheFullWidth) {
  EXPECT_EQ(base::CountRightZero(static_cast<u32>(0)), 32);
  EXPECT_EQ(base::CountRightZero(static_cast<u64>(0)), 64);
}

// The bucket allocator derives its size class from CountLeftZero, so the
// mapping it depends on is worth pinning here rather than only there.
TEST(BuiltinsBit, RoundUpToPowerOfTwoShapeUsedBySizeClasses) {
  constexpr u64 kCases[] = {1, 2, 3, 8, 9, 64, 65, 1024};
  constexpr int kExpectedShift[] = {0, 1, 2, 3, 4, 6, 7, 10};
  for (unsigned i = 0; i < _countof(kCases); i++) {
    const u64 value = kCases[i];
    const int shift =
        value <= 1 ? 0 : 64 - base::CountLeftZero(static_cast<u64>(value - 1));
    EXPECT_EQ(shift, kExpectedShift[i]) << "value " << value;
  }
}

// The width comes from sizeof, not MinMax<T>::digits(), which follows
// std::numeric_limits and reports one bit fewer for signed types. Deriving it
// from digits() made every u8/u16 answer off by nothing but would have made a
// signed one off by one, so the counters take unsigned only -- and the widths
// below are what proves the right source was used.
TEST(BuiltinsBit, WidthComesFromSizeofNotNumericLimitsDigits) {
  static_assert(base::MinMax<u8>::digits() == 8);
  static_assert(base::MinMax<i8>::digits() == 7);

  EXPECT_EQ(base::CountLeftZero(static_cast<u8>(0)), 8);
  EXPECT_EQ(base::CountLeftZero(static_cast<u8>(0x80)), 0);
  EXPECT_EQ(base::CountLeftZero(static_cast<u16>(0x8000)), 0);
  EXPECT_EQ(base::CountRightZero(static_cast<u8>(0x80)), 7);
  EXPECT_EQ(base::CountRightZero(static_cast<u16>(0x8000)), 15);
}

}  // namespace
