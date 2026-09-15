// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// ToString used to be snprintf calls; it now goes through base::FormatTo.
// These tests check it against snprintf directly, so the move is provably
// output-identical rather than merely believed to be.

#include <gtest/gtest.h>

#include <stdio.h>
#include <string.h>

#include <base/arch.h>
#include <base/numeric_limits.h>
#include <base/strings/to_string.h>

namespace {

template <typename T>
void ExpectMatchesPrintf(T value, const char* spec) {
  char expected[64];
  ::snprintf(expected, sizeof(expected), spec, value);
  const base::String actual = base::ToString(value);
  EXPECT_STREQ(actual.c_str(), expected) << "for spec " << spec;
  EXPECT_EQ(actual.length(), ::strlen(expected));
}

TEST(ToString, SignedIntegers) {
  for (int v : {0, 1, -1, 7, -7, 42, -42, 1000000, -1000000})
    ExpectMatchesPrintf(v, "%d");
  ExpectMatchesPrintf(base::MinMax<int>::max(), "%d");
  ExpectMatchesPrintf(base::MinMax<int>::min(), "%d");
}

TEST(ToString, UnsignedIntegers) {
  for (unsigned v : {0u, 1u, 42u, 65535u, 4000000000u})
    ExpectMatchesPrintf(v, "%u");
  ExpectMatchesPrintf(base::MinMax<unsigned>::max(), "%u");
}

TEST(ToString, LongAndLongLong) {
  ExpectMatchesPrintf(0L, "%ld");
  ExpectMatchesPrintf(-1L, "%ld");
  ExpectMatchesPrintf(base::MinMax<long>::max(), "%ld");
  ExpectMatchesPrintf(base::MinMax<long>::min(), "%ld");

  ExpectMatchesPrintf(0ULL, "%llu");
  ExpectMatchesPrintf(base::MinMax<unsigned long long>::max(), "%llu");

  ExpectMatchesPrintf(0LL, "%lld");
  ExpectMatchesPrintf(base::MinMax<long long>::max(), "%lld");
  // The one that breaks a naive negate-then-format: -min has no positive
  // counterpart in the type.
  ExpectMatchesPrintf(base::MinMax<long long>::min(), "%lld");
}

TEST(ToString, Floats) {
  for (float v : {0.0f, 1.0f, -1.0f, 3.25f, -3.25f, 100.5f})
    ExpectMatchesPrintf(v, "%.1f");
}

TEST(ToString, Doubles) {
  for (double v : {0.0, 1.0, -1.0, 3.14159, -3.14159, 2.5, 1234.5678})
    ExpectMatchesPrintf(v, "%.2f");
}

TEST(ToString, LengthMatchesContent) {
  const base::String s = base::ToString(-123456);
  EXPECT_EQ(s.length(), 7u);
  EXPECT_STREQ(s.c_str(), "-123456");
}

// Sweeps a wide spread of magnitudes and both signs, which a handful of
// literals would not.
TEST(ToString, MatchesPrintfAcrossMagnitudes) {
  long long value = 1;
  for (int i = 0; i < 19; i++) {
    ExpectMatchesPrintf(value, "%lld");
    ExpectMatchesPrintf(-value, "%lld");
    ExpectMatchesPrintf(value - 1, "%lld");
    ExpectMatchesPrintf(static_cast<unsigned long long>(value), "%llu");
    value *= 10;
  }
}

}  // namespace
