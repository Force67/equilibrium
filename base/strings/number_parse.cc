// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/strings/number_parse.h>

#include <base/memory/mem_ops.h>
#include <base/numeric_limits.h>
#include <base/strings/decimal_bignum.h>

namespace base {
namespace {

using numeric_detail::BigUInt;

bool IsSpace(char c) noexcept {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' ||
         c == '\r';
}

char Lower(char c) noexcept {
  return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// The digit |c| stands for in |radix|, or -1.
int DigitValue(char c, int radix) noexcept {
  int value;
  if (c >= '0' && c <= '9')
    value = c - '0';
  else if (c >= 'a' && c <= 'z')
    value = c - 'a' + 10;
  else if (c >= 'A' && c <= 'Z')
    value = c - 'A' + 10;
  else
    return -1;
  return value < radix ? value : -1;
}

struct Prefix {
  const char* cursor;
  bool negative;
};

Prefix SkipSpaceAndSign(const char* text) noexcept {
  const char* p = text;
  while (IsSpace(*p))
    p++;
  bool negative = false;
  if (*p == '+' || *p == '-') {
    negative = *p == '-';
    p++;
  }
  return {p, negative};
}

// Consumes the 0x / leading-0 radix prefix, resolving |radix| when it is 0.
const char* ApplyRadixPrefix(const char* p, int& radix) noexcept {
  if ((radix == 0 || radix == 16) && p[0] == '0' && Lower(p[1]) == 'x' &&
      DigitValue(p[2], 16) >= 0) {
    radix = 16;
    return p + 2;
  }
  if (radix == 0) {
    radix = (p[0] == '0' && DigitValue(p[1], 8) >= 0) ? 8 : 10;
  }
  return p;
}

// Accumulates digits in |radix| into |out|, saturating at |limit|. Reports
// whether any digit was seen.
bool AccumulateDigits(const char*& p, int radix, u64 limit, u64& out,
                      bool& saturated) noexcept {
  const u64 cutoff = limit / static_cast<u64>(radix);
  const u64 cutoff_digit = limit % static_cast<u64>(radix);
  u64 value = 0;
  bool any = false;
  saturated = false;
  for (;; p++) {
    const int digit = DigitValue(*p, radix);
    if (digit < 0)
      break;
    any = true;
    if (saturated)
      continue;  // Keep consuming so |end| lands past the whole number.
    if (value > cutoff ||
        (value == cutoff && static_cast<u64>(digit) > cutoff_digit)) {
      saturated = true;
      value = limit;
      continue;
    }
    value = value * static_cast<u64>(radix) + static_cast<u64>(digit);
  }
  out = value;
  return any;
}

// ── Decimal to binary ──────────────────────────────────────────────

// Assembles the double nearest to |mantissa| * 2^|exponent2|, where
// |mantissa| is an exact integer and |sticky| says whether nonzero bits were
// discarded below it.
//
// Rounding happens exactly once, at the precision the result can actually
// hold. Rounding to 53 bits first and then shifting into the subnormal range
// would round twice and land a ulp off. The two regimes are written out
// separately because a carry means different things in each: a normal number
// gains an exponent, while a subnormal's exponent is pinned at 2^-1074 and
// the significand simply grows.
f64 AssembleDouble(const BigUInt& mantissa, int exponent2, bool sticky,
                   bool negative) noexcept {
  const u64 sign_bit = negative ? (1ull << 63) : 0;
  auto from_bits = [](u64 bits) {
    f64 result;
    MemCopy(&result, &bits, sizeof(result));
    return result;
  };

  const int length = mantissa.BitLength();
  if (length == 0)
    return from_bits(sign_bit);

  // Power of two of the leading set bit: the value lies in [2^msb, 2^msb+1).
  const int msb = (length - 1) + exponent2;

  // Reads the |keep| bits down to 2^|lowest|, with the round bit and sticky
  // bit that sit below them, and rounds half-to-even. Returns the rounded
  // significand, which may have carried into one extra bit.
  auto round_at = [&](int lowest, int keep) -> u64 {
    const int cut = lowest - exponent2;
    u64 significand;
    bool round_bit = false;
    bool below = sticky;
    if (cut <= 0) {
      bool unused = false;
      significand = mantissa.HighBits(length, unused);
      significand <<= -cut;
    } else {
      bool dropped = false;
      significand = mantissa.HighBits(keep, dropped);
      round_bit = mantissa.BitAt(cut - 1);
      below = below || mantissa.AnyBitBelow(cut - 1);
    }
    if (round_bit && (below || (significand & 1)))
      significand++;
    return significand;
  };

  // ── Subnormal regime: the exponent is pinned, the significand floats.
  if (msb - 52 < -1074) {
    const int keep = msb - (-1074) + 1;
    if (keep <= 0) {
      // Below the smallest subnormal. It still rounds up to it when the value
      // is more than half of one; an exact half ties to even, which is zero.
      if (msb < -1075)
        return from_bits(sign_bit);
      const bool above_half = length > 1 || sticky;
      return from_bits(sign_bit | (above_half ? 1ull : 0ull));
    }
    // A carry here can reach 2^52, which written into the fraction field
    // overflows into an exponent of 1 -- exactly the smallest normal, and the
    // right answer assembled for free.
    return from_bits(sign_bit | round_at(-1074, keep));
  }

  // ── Normal regime: 53 significant bits, exponent follows the leading one.
  u64 significand = round_at(msb - 52, 53);
  int exponent = msb;
  if (significand >> 53) {  // carried out of 53 bits
    significand >>= 1;
    exponent++;
  }

  if (exponent > 1023)
    return from_bits(sign_bit | (0x7FFull << 52));  // infinity

  const u64 biased = static_cast<u64>(exponent + 1023);
  return from_bits(sign_bit | (biased << 52) |
                   (significand & ((1ull << 52) - 1)));
}

// A double can need at most 768 significant decimal digits to be rounded
// correctly; past that the only thing the digits can contribute is that the
// value is not an exact tie, which the sticky flag already records.
constexpr int kMaxSignificantDigits = 800;

f64 DecimalToDouble(const BigUInt& digits, int decimal_exponent, bool sticky,
                    bool negative) noexcept {
  BigUInt value = digits;
  if (value.IsZero())
    return AssembleDouble(value, 0, false, negative);

  if (decimal_exponent >= 0) {
    // Exact integer: multiply the tens in and hand it over.
    int power = decimal_exponent;
    while (power >= 9) {
      value.MulSmall(1000000000u);
      power -= 9;
    }
    static constexpr u32 kPow10[9] = {1,      10,      100,      1000,   10000,
                                      100000, 1000000, 10000000, 100000000};
    if (power)
      value.MulSmall(kPow10[power]);
    return AssembleDouble(value, 0, sticky, negative);
  }

  // value / 10^power. Scale up first so the quotient still carries 54
  // significant bits, then divide, folding every remainder into the sticky
  // bit -- which is all the discarded part can contribute to the rounding.
  const int power = -decimal_exponent;
  // log2(10) is a little over 3.32; the slack plus 64 guard bits is ample.
  const int shift = static_cast<int>(power * 3.3219280948873626) + 64;
  value.ShiftLeft(shift);

  int remaining = power;
  while (remaining > 0) {
    const int chunk = remaining >= 9 ? 9 : remaining;
    static constexpr u32 kPow10[10] = {1,       10,       100,      1000,
                                       10000,   100000,   1000000,  10000000,
                                       100000000, 1000000000u};
    if (value.DivModSmall(kPow10[chunk]) != 0)
      sticky = true;
    remaining -= chunk;
  }

  return AssembleDouble(value, -shift, sticky, negative);
}

bool MatchWord(const char*& p, const char* word) noexcept {
  const char* q = p;
  while (*word) {
    if (Lower(*q) != *word)
      return false;
    q++;
    word++;
  }
  p = q;
  return true;
}

}  // namespace

bool ParseInteger(const char* text, i64& out, int base_radix,
                  const char** end) noexcept {
  if (!text)
    return false;
  const Prefix prefix = SkipSpaceAndSign(text);
  int radix = base_radix;
  const char* p = ApplyRadixPrefix(prefix.cursor, radix);
  if (radix < 2 || radix > 36)
    return false;

  // Negative range is one wider than positive, so accumulate against the
  // limit that actually applies; LLONG_MIN has no positive counterpart.
  const u64 limit = prefix.negative
                        ? (static_cast<u64>(MinMax<i64>::max()) + 1)
                        : static_cast<u64>(MinMax<i64>::max());
  u64 magnitude = 0;
  bool saturated = false;
  if (!AccumulateDigits(p, radix, limit, magnitude, saturated))
    return false;

  if (end)
    *end = p;
  out = prefix.negative ? static_cast<i64>(~magnitude + 1)
                        : static_cast<i64>(magnitude);
  return true;
}

bool ParseUnsigned(const char* text, u64& out, int base_radix,
                   const char** end) noexcept {
  if (!text)
    return false;
  const Prefix prefix = SkipSpaceAndSign(text);
  int radix = base_radix;
  const char* p = ApplyRadixPrefix(prefix.cursor, radix);
  if (radix < 2 || radix > 36)
    return false;

  u64 magnitude = 0;
  bool saturated = false;
  if (!AccumulateDigits(p, radix, MinMax<u64>::max(), magnitude, saturated))
    return false;

  if (end)
    *end = p;
  // strtoull negates in unsigned arithmetic rather than rejecting a sign.
  out = prefix.negative ? (~magnitude + 1) : magnitude;
  return true;
}

bool ParseFloat(const char* text, f64& out, const char** end) noexcept {
  if (!text)
    return false;
  const Prefix prefix = SkipSpaceAndSign(text);
  const char* p = prefix.cursor;

  if (MatchWord(p, "inf")) {
    MatchWord(p, "inity");  // the long spelling is optional
    if (end)
      *end = p;
    const u64 bits = (prefix.negative ? (1ull << 63) : 0) | (0x7FFull << 52);
    MemCopy(&out, &bits, sizeof(out));
    return true;
  }
  if (MatchWord(p, "nan")) {
    if (end)
      *end = p;
    const u64 bits = (prefix.negative ? (1ull << 63) : 0) | (0x7FFull << 52) |
                     (1ull << 51);
    MemCopy(&out, &bits, sizeof(out));
    return true;
  }

  BigUInt digits;
  int significant = 0;
  int exponent = 0;
  bool any_digit = false;
  bool sticky = false;

  // Integer part.
  for (; *p >= '0' && *p <= '9'; p++) {
    any_digit = true;
    if (digits.IsZero() && *p == '0')
      continue;  // leading zeros carry no information
    if (significant < kMaxSignificantDigits) {
      digits.MulAddSmall(10, static_cast<u32>(*p - '0'));
      significant++;
    } else {
      // Past the cap the digit only shifts the value; record any nonzero one
      // so an exact tie cannot be mistaken for one.
      exponent++;
      if (*p != '0')
        sticky = true;
    }
  }

  // Fraction.
  if (*p == '.') {
    p++;
    for (; *p >= '0' && *p <= '9'; p++) {
      any_digit = true;
      if (digits.IsZero() && *p == '0') {
        exponent--;  // still scales the value, just adds no digit
        continue;
      }
      if (significant < kMaxSignificantDigits) {
        digits.MulAddSmall(10, static_cast<u32>(*p - '0'));
        significant++;
        exponent--;
      } else if (*p != '0') {
        sticky = true;
      }
    }
  }

  if (!any_digit)
    return false;

  // Exponent.
  if (Lower(*p) == 'e') {
    const char* after = p + 1;
    bool exponent_negative = false;
    if (*after == '+' || *after == '-') {
      exponent_negative = *after == '-';
      after++;
    }
    if (*after >= '0' && *after <= '9') {
      i64 magnitude = 0;
      for (; *after >= '0' && *after <= '9'; after++) {
        if (magnitude < 100000)  // far past any usable range; stop growing
          magnitude = magnitude * 10 + (*after - '0');
      }
      exponent += static_cast<int>(exponent_negative ? -magnitude : magnitude);
      p = after;
    }
  }

  if (end)
    *end = p;

  // Short-circuit the ranges where the answer cannot be anything else, which
  // also keeps the shift below inside the bignum's capacity.
  const int magnitude_estimate = significant + exponent;
  if (digits.IsZero() || magnitude_estimate < -400) {
    const u64 bits = prefix.negative ? (1ull << 63) : 0;
    MemCopy(&out, &bits, sizeof(out));
    return true;
  }
  if (magnitude_estimate > 350) {
    const u64 bits = (prefix.negative ? (1ull << 63) : 0) | (0x7FFull << 52);
    MemCopy(&out, &bits, sizeof(out));
    return true;
  }

  out = DecimalToDouble(digits, exponent, sticky, prefix.negative);
  return true;
}

}  // namespace base
