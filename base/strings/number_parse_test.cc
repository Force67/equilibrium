// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Checked against strtoll/strtoull/strtod directly, including how far each
// consumed, so replacing them is provably behaviour-preserving for the inputs
// base actually sees.

#include <gtest/gtest.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <base/arch.h>
#include <base/numeric_limits.h>
#include <base/strings/number_parse.h>

namespace {

int SweepIterations(int fallback) {
  const char* setting = ::getenv("BASE_NUMBER_FUZZ_ITERATIONS");
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

testing::AssertionResult MatchesStrtoll(const char* text, int radix) {
  char* libc_end = nullptr;
  const long long expected = ::strtoll(text, &libc_end, radix);
  const bool libc_parsed = libc_end != text;

  i64 actual = 0;
  const char* base_end = nullptr;
  const bool parsed = base::ParseInteger(text, actual, radix, &base_end);

  if (parsed != libc_parsed) {
    return testing::AssertionFailure()
           << "'" << text << "' radix " << radix << ": strtoll "
           << (libc_parsed ? "parsed" : "rejected") << ", base "
           << (parsed ? "parsed" : "rejected");
  }
  if (!parsed)
    return testing::AssertionSuccess();
  if (actual != expected) {
    return testing::AssertionFailure()
           << "'" << text << "' radix " << radix << ": strtoll " << expected
           << ", base " << actual;
  }
  if (base_end != libc_end) {
    return testing::AssertionFailure()
           << "'" << text << "' radix " << radix << ": consumed "
           << (base_end - text) << ", strtoll consumed " << (libc_end - text);
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult MatchesStrtod(const char* text) {
  char* libc_end = nullptr;
  const double expected = ::strtod(text, &libc_end);
  const bool libc_parsed = libc_end != text;

  f64 actual = 0;
  const char* base_end = nullptr;
  const bool parsed = base::ParseFloat(text, actual, &base_end);

  if (parsed != libc_parsed) {
    return testing::AssertionFailure()
           << "'" << text << "': strtod " << (libc_parsed ? "parsed" : "rejected")
           << ", base " << (parsed ? "parsed" : "rejected");
  }
  if (!parsed)
    return testing::AssertionSuccess();

  u64 expected_bits, actual_bits;
  ::memcpy(&expected_bits, &expected, sizeof(expected_bits));
  ::memcpy(&actual_bits, &actual, sizeof(actual_bits));

  // A NaN's payload is unspecified, and the two libraries pick different
  // ones: glibc returns a bare quiet NaN for "nan" where the Microsoft CRT
  // sets every payload bit. Both are correct, so only the sign is compared
  // here, since strtod does carry that through from "-nan".
  const bool both_nan = expected != expected && actual != actual;
  if (both_nan) {
    if ((expected_bits >> 63) != (actual_bits >> 63)) {
      return testing::AssertionFailure()
             << "'" << text << "': NaN sign differs, strtod "
             << (expected_bits >> 63) << ", base " << (actual_bits >> 63);
    }
    if (base_end != libc_end) {
      return testing::AssertionFailure()
             << "'" << text << "': consumed " << (base_end - text)
             << ", strtod consumed " << (libc_end - text);
    }
    return testing::AssertionSuccess();
  }

  // Everything finite is compared bit for bit, so a one-ulp difference
  // cannot hide behind ==.
  if (expected_bits != actual_bits) {
    char detail[256];
    ::snprintf(detail, sizeof(detail),
               "strtod %.17g (%016llx), base %.17g (%016llx)", expected,
               static_cast<unsigned long long>(expected_bits), actual,
               static_cast<unsigned long long>(actual_bits));
    return testing::AssertionFailure() << "'" << text << "': " << detail;
  }
  if (base_end != libc_end) {
    return testing::AssertionFailure()
           << "'" << text << "': consumed " << (base_end - text)
           << ", strtod consumed " << (libc_end - text);
  }
  return testing::AssertionSuccess();
}

TEST(NumberParse, IntegerBasics) {
  for (const char* text : {"0", "1", "-1", "42", "-42", "  7", "+7",
                           "2147483647", "-2147483648", "9223372036854775807",
                           "-9223372036854775808"}) {
    EXPECT_TRUE(MatchesStrtoll(text, 10)) << text;
  }
}

TEST(NumberParse, IntegerRejectsNonNumbers) {
  for (const char* text : {"", "abc", "  ", "+", "-", ".5", "e5"})
    EXPECT_TRUE(MatchesStrtoll(text, 10)) << text;
}

TEST(NumberParse, IntegerStopsAtTrailingText) {
  for (const char* text : {"12abc", "5 6", "7,8", "9.5", "1e3"})
    EXPECT_TRUE(MatchesStrtoll(text, 10)) << text;
}

TEST(NumberParse, IntegerRadix) {
  for (const char* text : {"0x1f", "0X1F", "1f", "0777", "777", "101", "z"}) {
    for (int radix : {0, 2, 8, 10, 16, 36})
      EXPECT_TRUE(MatchesStrtoll(text, radix)) << text << " radix " << radix;
  }
}

TEST(NumberParse, IntegerSaturatesOnOverflow) {
  for (const char* text : {"99999999999999999999999",
                           "-99999999999999999999999",
                           "9223372036854775808",
                           "-9223372036854775809"}) {
    EXPECT_TRUE(MatchesStrtoll(text, 10)) << text;
  }
}

TEST(NumberParse, Unsigned) {
  u64 value = 0;
  EXPECT_TRUE(base::ParseUnsigned("18446744073709551615", value));
  EXPECT_EQ(value, base::MinMax<u64>::max());
  EXPECT_TRUE(base::ParseUnsigned("0", value));
  EXPECT_EQ(value, 0u);
  EXPECT_TRUE(base::ParseUnsigned("ff", value, 16));
  EXPECT_EQ(value, 255u);
  EXPECT_FALSE(base::ParseUnsigned("zzz", value, 10));
}

TEST(NumberParse, FloatBasics) {
  for (const char* text : {"0", "0.0", "-0.0", "1", "1.0", "-1.5", "3.14159",
                           "0.1", "0.2", "0.5", "100.25", "  2.5", "+2.5"}) {
    EXPECT_TRUE(MatchesStrtod(text)) << text;
  }
}

TEST(NumberParse, FloatScientific) {
  for (const char* text : {"1e0", "1e10", "1e-10", "1E5", "1e+5", "-2.5e-3",
                           "1.7976931348623157e308", "5e-324", "2.2250738585072014e-308",
                           "1e309", "1e-400", "123456789e-9"}) {
    EXPECT_TRUE(MatchesStrtod(text)) << text;
  }
}

TEST(NumberParse, FloatSpecials) {
  for (const char* text : {"inf", "INF", "-inf", "infinity", "-Infinity",
                           "nan", "NAN", "-nan"}) {
    EXPECT_TRUE(MatchesStrtod(text)) << text;
  }
}

TEST(NumberParse, FloatRejectsNonNumbers) {
  for (const char* text : {"", "abc", "  ", "+", "-", ".", "e5", "-.e3"})
    EXPECT_TRUE(MatchesStrtod(text)) << text;
}

TEST(NumberParse, FloatStopsAtTrailingText) {
  for (const char* text : {"1.5abc", "2.5 ", "3.5,", "1e", "1e+", "1.2.3"})
    EXPECT_TRUE(MatchesStrtod(text)) << text;
}

// Long digit strings are where a parser that multiplies up in floating point
// accumulates error; these must still land on the exact nearest double.
TEST(NumberParse, FloatLongDigitStrings) {
  for (const char* text : {
           "0.10000000000000000555111512312578270211815834045410156250",
           "2.22507385850720138309023271733240406421921598046233183055332741688720443481391819585428315901251102056406733973103581100515243416155346010885601238537771882113077799353200233047961014744258363607192156504694250373420837525080665061665815894872049117996859163964850063590877011830487479978088775374994945158045160505091539985658247081864511353793580499211598108576605199243335211435239014879569960959128889160299264151106346631339366347758651302937176204732563178148566435087212282863764204484681140761391147706280168985324411002416144742161856716615054015428508471675290190316132277889672970737312333408698898317506783884692609277397797285865965494109136909540613646756870239867831529068098461721092462539672851562500000000000000000000e-308",
           "1.00000000000000000000000000000000000000000000000000001e0",
           "9999999999999999999999999999999999999999999999999999999999e-30",
       }) {
    EXPECT_TRUE(MatchesStrtod(text)) << "long input";
  }
}

// Exactly halfway between two doubles: the tie-breaking rule has to match.
TEST(NumberParse, FloatExactTies) {
  for (const char* text : {"1.0000000000000002", "1.0000000000000001",
                           "4503599627370497.5", "9007199254740993",
                           "2.00000000000000011102230246251565404236316680908203125"}) {
    EXPECT_TRUE(MatchesStrtod(text)) << text;
  }
}

// ── Sweeps ──────────────────────────────────────────────────────────

// Round-trips every double through its own %.17g text, which is the shortest
// form guaranteed to reproduce it. Any rounding error shows as a bit change.
TEST(NumberParse, SweepsRoundTripRandomDoubles) {
  Xorshift random(0xfeedfaceull);
  const int iterations = SweepIterations(200000);
  for (int i = 0; i < iterations; i++) {
    const u64 bits = random.Next();
    f64 value;
    ::memcpy(&value, &bits, sizeof(value));
    if (value != value || value - value != 0)
      continue;
    char text[64];
    ::snprintf(text, sizeof(text), "%.17g", value);
    ASSERT_TRUE(MatchesStrtod(text));
  }
}

TEST(NumberParse, SweepsRandomDecimalTextAgainstStrtod) {
  Xorshift random(0x0badc0deull);
  const int iterations = SweepIterations(200000);
  for (int i = 0; i < iterations; i++) {
    char text[80];
    const int digits = 1 + static_cast<int>(random.Next() % 25);
    int at = 0;
    if (random.Next() & 1)
      text[at++] = '-';
    for (int d = 0; d < digits; d++) {
      text[at++] = static_cast<char>('0' + (random.Next() % 10));
      if (d == 0 && (random.Next() & 1))
        text[at++] = '.';
    }
    const int exponent = static_cast<int>(random.Next() % 700) - 350;
    at += ::snprintf(text + at, sizeof(text) - at, "e%d", exponent);
    text[at] = '\0';
    ASSERT_TRUE(MatchesStrtod(text));
  }
}

TEST(NumberParse, SweepsRandomIntegerTextAgainstStrtoll) {
  Xorshift random(0x13572468ull);
  const int iterations = SweepIterations(200000);
  for (int i = 0; i < iterations; i++) {
    char text[40];
    const int digits = 1 + static_cast<int>(random.Next() % 22);
    int at = 0;
    if (random.Next() & 1)
      text[at++] = '-';
    for (int d = 0; d < digits; d++)
      text[at++] = static_cast<char>('0' + (random.Next() % 10));
    text[at] = '\0';
    ASSERT_TRUE(MatchesStrtoll(text, 10));
  }
}

}  // namespace
