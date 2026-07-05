// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "time.h"

namespace {
TEST(TimeDelta, FactoriesConvert) {
  EXPECT_EQ(base::Microseconds(42).InMicroseconds(), 42);
  EXPECT_EQ(base::Milliseconds(1500).InMilliseconds(), 1500);
  EXPECT_EQ(base::Milliseconds(1500).InSeconds(), 1);
  EXPECT_EQ(base::Seconds(2).InMicroseconds(), 2000000);
  EXPECT_EQ(base::Minutes(2).InSeconds(), 120);
  EXPECT_DOUBLE_EQ(base::Milliseconds(500).InSecondsF(), 0.5);
}

TEST(TimeDelta, Arithmetic) {
  base::TimeDelta delta = base::Seconds(1) + base::Milliseconds(500);
  EXPECT_EQ(delta.InMilliseconds(), 1500);
  delta -= base::Milliseconds(500);
  EXPECT_EQ(delta.InMilliseconds(), 1000);
  EXPECT_EQ((-base::Seconds(1)).InSeconds(), -1);
  EXPECT_TRUE(base::Seconds(1) > base::Milliseconds(999));
  EXPECT_TRUE(base::TimeDelta().is_zero());
}

TEST(TimeTicks, IsMonotonic) {
  const base::TimeTicks first = base::TimeTicks::Now();
  EXPECT_FALSE(first.is_null());

  base::TimeTicks last = first;
  for (int i = 0; i < 1000; i++) {
    const base::TimeTicks now = base::TimeTicks::Now();
    EXPECT_GE(now, last);
    last = now;
  }
  EXPECT_GE(last - first, base::TimeDelta());
}

TEST(TimeTicks, DeltaArithmetic) {
  const base::TimeTicks now = base::TimeTicks::Now();
  const base::TimeTicks later = now + base::Seconds(1);
  EXPECT_EQ((later - now).InMilliseconds(), 1000);
  EXPECT_EQ(later - base::Seconds(1), now);
}

TEST(Time, NowMatchesUnixTimeStamp) {
  const base::Time now = base::Time::Now();
  const i64 stamp = base::GetUnixTimeStamp();
  const i64 now_seconds = (now - base::Time()).InSeconds();
  EXPECT_GE(now_seconds, stamp - 2);
  EXPECT_LE(now_seconds, stamp + 2);
}
}  // namespace
