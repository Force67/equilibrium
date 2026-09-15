// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::FormatFloatTo has to agree with printf, so that is what these tests
// check: the same value and spec through both, byte for byte. The sweeps at
// the bottom cover the bit patterns that hand-picked literals never reach --
// subnormals, the exponent boundaries, exact ties, and a large random sample.

#include <gtest/gtest.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <base/arch.h>
#include <base/strings/float_format.h>

namespace {

// A local generator, not base::RandomUint, so a failure reproduces exactly
// from the seed printed beside it.
// CI runs the default sweep; BASE_FLOAT_FUZZ_ITERATIONS raises it for a deep
// run, which is how the converter was first shaken out.
int SweepIterations(int fallback) {
  const char* setting = ::getenv("BASE_FLOAT_FUZZ_ITERATIONS");
  if (!setting)
    return fallback;
  const long parsed = ::strtol(setting, nullptr, 10);
  return parsed > 0 ? static_cast<int>(parsed) : fallback;
}

class Xorshift {
 public:
  explicit Xorshift(u64 seed) noexcept : state_(seed ? seed : 1) {}
  u64 Next() noexcept {
    state_ ^= state_ << 13;
    state_ ^= state_ >> 7;
    state_ ^= state_ << 17;
    return state_;
  }

 private:
  u64 state_;
};

f64 FromBits(u64 bits) {
  f64 value;
  ::memcpy(&value, &bits, sizeof(value));
  return value;
}

// Builds the equivalent printf spec and compares. Returns false on a
// mismatch, having already recorded the failure.
testing::AssertionResult Matches(f64 value, char type, int precision,
                                 char sign = '\0', bool alt = false) {
  char spec[16];
  int at = 0;
  spec[at++] = '%';
  if (sign)
    spec[at++] = sign;
  if (alt)
    spec[at++] = '#';
  if (precision >= 0)
    at += ::snprintf(spec + at, sizeof(spec) - at, ".%d", precision);
  spec[at++] = type;
  spec[at] = '\0';

  char expected[1100];
  ::snprintf(expected, sizeof(expected), spec, value);

  char actual[1100];
  const mem_size length =
      base::FormatFloatTo(actual, sizeof(actual), value, type, precision, sign,
                          alt);
  if (length >= sizeof(actual))
    return testing::AssertionFailure() << "output too long: " << length;
  actual[length] = '\0';

  if (::strcmp(actual, expected) == 0)
    return testing::AssertionSuccess();

  char bits_text[32];
  u64 bits;
  ::memcpy(&bits, &value, sizeof(bits));
  ::snprintf(bits_text, sizeof(bits_text), "%016llx",
             static_cast<unsigned long long>(bits));
  return testing::AssertionFailure()
         << "spec " << spec << " on 0x" << bits_text << " (" << value << ")\n"
         << "  printf: '" << expected << "'\n"
         << "  base:   '" << actual << "'";
}

TEST(FloatFormat, FixedBasics) {
  for (int p = 0; p <= 8; p++) {
    EXPECT_TRUE(Matches(0.0, 'f', p));
    EXPECT_TRUE(Matches(-0.0, 'f', p));
    EXPECT_TRUE(Matches(1.0, 'f', p));
    EXPECT_TRUE(Matches(-1.0, 'f', p));
    EXPECT_TRUE(Matches(0.5, 'f', p));
    EXPECT_TRUE(Matches(3.14159265358979, 'f', p));
    EXPECT_TRUE(Matches(-2.718281828459045, 'f', p));
    EXPECT_TRUE(Matches(1234567.891, 'f', p));
  }
}

TEST(FloatFormat, FixedDefaultPrecision) {
  EXPECT_TRUE(Matches(1.0, 'f', -1));
  EXPECT_TRUE(Matches(0.1, 'f', -1));
  EXPECT_TRUE(Matches(123.456, 'f', -1));
}

// 0.1 and friends are not the decimal they look like; printf prints the exact
// binary value, and so must this.
TEST(FloatFormat, ExactBinaryExpansionNotTheLiteral) {
  EXPECT_TRUE(Matches(0.1, 'f', 20));
  EXPECT_TRUE(Matches(0.1, 'f', 30));
  EXPECT_TRUE(Matches(0.2, 'f', 25));
  EXPECT_TRUE(Matches(1.0 / 3.0, 'f', 40));
  EXPECT_TRUE(Matches(2.675, 'f', 20));
}

TEST(FloatFormat, RoundingHalfToEven) {
  // Values whose decimal expansion ends in an exact 5 at the cut.
  EXPECT_TRUE(Matches(0.5, 'f', 0));
  EXPECT_TRUE(Matches(1.5, 'f', 0));
  EXPECT_TRUE(Matches(2.5, 'f', 0));
  EXPECT_TRUE(Matches(3.5, 'f', 0));
  EXPECT_TRUE(Matches(0.125, 'f', 2));
  EXPECT_TRUE(Matches(0.375, 'f', 2));
  EXPECT_TRUE(Matches(-2.5, 'f', 0));
}

TEST(FloatFormat, RoundingCarriesThroughNines) {
  EXPECT_TRUE(Matches(0.999999, 'f', 3));
  EXPECT_TRUE(Matches(9.9999999, 'f', 2));
  EXPECT_TRUE(Matches(99.995, 'f', 2));
  EXPECT_TRUE(Matches(0.0999, 'f', 2));
  EXPECT_TRUE(Matches(9.999, 'e', 2));
}

// The whole value sits below the requested position.
TEST(FloatFormat, RoundsAwayEntirely) {
  EXPECT_TRUE(Matches(0.0001, 'f', 2));
  EXPECT_TRUE(Matches(0.004, 'f', 2));
  EXPECT_TRUE(Matches(0.006, 'f', 2));
  EXPECT_TRUE(Matches(0.05, 'f', 1));
  EXPECT_TRUE(Matches(0.04, 'f', 1));
  EXPECT_TRUE(Matches(1e-300, 'f', 2));
}

TEST(FloatFormat, Scientific) {
  for (int p = 0; p <= 8; p++) {
    EXPECT_TRUE(Matches(0.0, 'e', p));
    EXPECT_TRUE(Matches(1.0, 'e', p));
    EXPECT_TRUE(Matches(-1.5, 'e', p));
    EXPECT_TRUE(Matches(1234.5678, 'e', p));
    EXPECT_TRUE(Matches(1e-10, 'e', p));
    EXPECT_TRUE(Matches(1e300, 'e', p));
    EXPECT_TRUE(Matches(-4.94e-324, 'e', p));
  }
}

// printf pads the exponent to two digits and lets it run to three.
TEST(FloatFormat, ScientificExponentWidth) {
  EXPECT_TRUE(Matches(1e5, 'e', 2));
  EXPECT_TRUE(Matches(1e-5, 'e', 2));
  EXPECT_TRUE(Matches(1e99, 'e', 2));
  EXPECT_TRUE(Matches(1e100, 'e', 2));
  EXPECT_TRUE(Matches(1e-100, 'e', 2));
  EXPECT_TRUE(Matches(1e308, 'e', 2));
  EXPECT_TRUE(Matches(5e-324, 'e', 2));
}

TEST(FloatFormat, General) {
  for (int p = -1; p <= 10; p++) {
    EXPECT_TRUE(Matches(0.0, 'g', p));
    EXPECT_TRUE(Matches(1.0, 'g', p));
    EXPECT_TRUE(Matches(100.0, 'g', p));
    EXPECT_TRUE(Matches(0.0001, 'g', p));
    EXPECT_TRUE(Matches(0.00001, 'g', p));
    EXPECT_TRUE(Matches(123456.0, 'g', p));
    EXPECT_TRUE(Matches(1234567.0, 'g', p));
    EXPECT_TRUE(Matches(3.14159265358979, 'g', p));
    EXPECT_TRUE(Matches(-0.000123456, 'g', p));
  }
}

TEST(FloatFormat, NonFinite) {
  // Built from bits rather than 1.0 / 0.0, which MSVC rejects outright as a
  // constant divide by zero instead of folding it to infinity.
  const f64 inf = FromBits(0x7FF0000000000000ull);
  const f64 nan = FromBits(0x7FF8000000000000ull);
  for (char type : {'f', 'e', 'g', 'F', 'E', 'G'}) {
    EXPECT_TRUE(Matches(inf, type, 2));
    EXPECT_TRUE(Matches(-inf, type, 2));
    EXPECT_TRUE(Matches(nan, type, 2));
  }
}

TEST(FloatFormat, SignFlags) {
  for (char sign : {'+', ' '}) {
    EXPECT_TRUE(Matches(1.5, 'f', 2, sign));
    EXPECT_TRUE(Matches(-1.5, 'f', 2, sign));
    EXPECT_TRUE(Matches(0.0, 'f', 2, sign));
    EXPECT_TRUE(Matches(1.5, 'e', 2, sign));
    EXPECT_TRUE(Matches(1.5, 'g', 4, sign));
  }
}

TEST(FloatFormat, AlternateForm) {
  EXPECT_TRUE(Matches(1.0, 'f', 0, '\0', /*alt=*/true));
  EXPECT_TRUE(Matches(1.0, 'e', 0, '\0', /*alt=*/true));
  EXPECT_TRUE(Matches(1.0, 'g', 6, '\0', /*alt=*/true));
  EXPECT_TRUE(Matches(100.0, 'g', 6, '\0', /*alt=*/true));
}

TEST(FloatFormat, Subnormals) {
  const f64 smallest = 4.9406564584124654e-324;
  EXPECT_TRUE(Matches(smallest, 'e', 17));
  EXPECT_TRUE(Matches(smallest, 'g', 17));
  EXPECT_TRUE(Matches(smallest * 2, 'e', 17));
  EXPECT_TRUE(Matches(2.2250738585072009e-308, 'e', 17));  // largest subnormal
  EXPECT_TRUE(Matches(2.2250738585072014e-308, 'e', 17));  // smallest normal
}

TEST(FloatFormat, Extremes) {
  EXPECT_TRUE(Matches(1.7976931348623157e308, 'e', 17));
  EXPECT_TRUE(Matches(1.7976931348623157e308, 'f', 2));
  EXPECT_TRUE(Matches(1.7976931348623157e308, 'g', 17));
}

// The output the caller sizes a buffer from must be the true length even when
// it did not fit, as snprintf reports.
TEST(FloatFormat, ReportsLengthWhenTruncated) {
  char small[4];
  const mem_size needed =
      base::FormatFloatTo(small, sizeof(small), 3.14159, 'f', 4, '\0', false);
  EXPECT_EQ(needed, 6u);  // "3.1416"
  char full[16];
  const mem_size again =
      base::FormatFloatTo(full, sizeof(full), 3.14159, 'f', 4, '\0', false);
  EXPECT_EQ(again, needed);
  EXPECT_EQ(::strncmp(full, "3.1416", 6), 0);
}

// ── Sweeps ──────────────────────────────────────────────────────────

TEST(FloatFormat, SweepsEveryExponentAgainstPrintf) {
  // One value per binary exponent, across the whole range, at several
  // precisions in each layout.
  for (int e = -1074; e <= 1023; e++) {
    const f64 value = ::ldexp(1.0, e);
    for (int p : {0, 1, 6, 17}) {
      ASSERT_TRUE(Matches(value, 'e', p)) << "exponent " << e;
      ASSERT_TRUE(Matches(value, 'g', p == 0 ? 1 : p)) << "exponent " << e;
    }
    ASSERT_TRUE(Matches(value, 'f', 6)) << "exponent " << e;
  }
}

TEST(FloatFormat, SweepsRandomBitPatternsAgainstPrintf) {
  Xorshift random(0x5eed1234ull);
  const int iterations = SweepIterations(200000);
  for (int i = 0; i < iterations; i++) {
    const u64 bits = random.Next();
    f64 value;
    ::memcpy(&value, &bits, sizeof(value));
    if (value != value || value - value != 0)
      continue;  // non-finite, covered separately
    const int precision = static_cast<int>(random.Next() % 19);
    ASSERT_TRUE(Matches(value, 'e', precision));
    ASSERT_TRUE(Matches(value, 'g', precision == 0 ? 1 : precision));
    ASSERT_TRUE(Matches(value, 'f', precision));
  }
}

TEST(FloatFormat, SweepsSmallDecimalsAgainstPrintf) {
  // Values near ties, where the rounding decision is hardest.
  Xorshift random(0xabcdef01ull);
  const int iterations = SweepIterations(100000);
  for (int i = 0; i < iterations; i++) {
    const f64 value = static_cast<f64>(random.Next() % 1000000) /
                      static_cast<f64>(1ull << (random.Next() % 20));
    for (int p : {0, 1, 2, 3, 6}) {
      ASSERT_TRUE(Matches(value, 'f', p));
      ASSERT_TRUE(Matches(-value, 'f', p));
    }
  }
}

}  // namespace
