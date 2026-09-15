// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/strings/float_format.h>

#include <base/memory/mem_ops.h>
#include <base/strings/decimal_bignum.h>

namespace base {
namespace {

using numeric_detail::BigUInt;

// ── Exact decimal expansion ────────────────────────────────────────
//
// A finite double is m * 2^e with m a 53-bit integer. For e >= 0 that is the
// integer m << e. For e < 0 it is m / 2^-e, and multiplying numerator and
// denominator by 5^-e turns the denominator into 10^-e:
//
//     m / 2^-e  ==  m * 5^-e / 10^-e
//
// so the exact decimal digits are those of the integer m * 5^-e, with -e of
// them falling after the point. No approximation is involved, which is why
// the rounding below can be a single exact decision.

// 5^13 is the largest power of five that fits a u32, so the multiply below
// runs in as few steps as a 32-bit limb allows.
constexpr u32 kPow5Chunk = 1220703125u;  // 5^13
constexpr int kPow5ChunkExp = 13;
constexpr u32 kPow5Table[kPow5ChunkExp] = {1,        5,       25,      125,
                                           625,      3125,    15625,   78125,
                                           390625,   1953125, 9765625, 48828125,
                                           244140625};

// The exact digits of a finite, non-zero double.
struct Decimal {
  // Significant digits, most significant first, as ASCII. The value is
  // 0.digits * 10^exponent.
  char digits[800];
  int count;
  int exponent;
};

void ExpandExact(u64 mantissa, int exponent2, Decimal& out) noexcept {
  BigUInt n;
  n.SetU64(mantissa);

  int fraction_digits = 0;
  if (exponent2 >= 0) {
    n.ShiftLeft(exponent2);
  } else {
    int power = -exponent2;
    fraction_digits = power;
    while (power >= kPow5ChunkExp) {
      n.MulSmall(kPow5Chunk);
      power -= kPow5ChunkExp;
    }
    if (power)
      n.MulSmall(kPow5Table[power]);
  }

  // Extract nine digits per division, least significant group first.
  char reversed[800];
  int length = 0;
  while (!n.IsZero()) {
    u32 group = n.DivModSmall(1000000000u);
    for (int i = 0; i < 9; i++) {
      reversed[length++] = static_cast<char>('0' + (group % 10));
      group /= 10;
    }
  }
  // The final group carries leading zeros that are not part of the number.
  while (length > 0 && reversed[length - 1] == '0')
    length--;

  for (int i = 0; i < length; i++)
    out.digits[i] = reversed[length - 1 - i];
  out.count = length;
  // value = N * 10^-fraction_digits, and N has `length` digits, so writing it
  // as 0.digits * 10^exponent puts the point `length - fraction_digits` in.
  out.exponent = length - fraction_digits;
}

// Rounds the expansion to |keep| significant digits, half-to-even against the
// exact remainder. |keep| may be zero or negative, meaning the whole value
// sits below the requested position and either vanishes or carries to a
// single 1.
void RoundTo(Decimal& value, int keep) noexcept {
  if (keep >= value.count)
    return;  // Already shorter than asked for; the caller pads with zeros.

  if (keep < 0) {
    value.count = 0;
    return;
  }

  // Decide on the first dropped digit, then on whether anything follows it.
  const char first_dropped = value.digits[keep];
  bool round_up;
  if (first_dropped > '5') {
    round_up = true;
  } else if (first_dropped < '5') {
    round_up = false;
  } else {
    bool rest_nonzero = false;
    for (int i = keep + 1; i < value.count; i++) {
      if (value.digits[i] != '0') {
        rest_nonzero = true;
        break;
      }
    }
    // An exact tie goes to the even last kept digit. With nothing kept, the
    // digit before the cut is an implicit 0, which is already even.
    round_up = rest_nonzero ||
               (keep > 0 && ((value.digits[keep - 1] - '0') & 1) != 0);
  }

  value.count = keep;
  if (!round_up)
    return;

  // Propagate the carry; 999 -> 1000 gains a digit and shifts the exponent.
  int i = keep - 1;
  for (; i >= 0; i--) {
    if (value.digits[i] != '9') {
      value.digits[i]++;
      return;
    }
    value.digits[i] = '0';
  }
  value.digits[0] = '1';
  value.count = keep > 0 ? keep : 1;
  value.exponent++;
}

class Writer {
 public:
  Writer(char* out, mem_size capacity) noexcept
      : out_(out), capacity_(capacity) {}

  void Put(char c) noexcept {
    if (length_ < capacity_)
      out_[length_] = c;
    length_++;
  }
  void Repeat(char c, int times) noexcept {
    for (int i = 0; i < times; i++)
      Put(c);
  }
  void PutAll(const char* text, int count) noexcept {
    for (int i = 0; i < count; i++)
      Put(text[i]);
  }
  mem_size length() const noexcept { return length_; }

 private:
  char* out_;
  mem_size capacity_;
  mem_size length_ = 0;
};

void WriteSign(Writer& writer, bool negative, char sign) noexcept {
  if (negative)
    writer.Put('-');
  else if (sign == '+' || sign == ' ')
    writer.Put(sign);
}

// d.dddd with |precision| digits after the point.
void WriteFixed(Writer& writer, const Decimal& value, int precision,
                bool alt) noexcept {
  if (value.exponent <= 0) {
    writer.Put('0');
  } else {
    for (int i = 0; i < value.exponent; i++)
      writer.Put(i < value.count ? value.digits[i] : '0');
  }

  if (precision <= 0) {
    if (alt)
      writer.Put('.');
    return;
  }

  writer.Put('.');
  for (int k = 1; k <= precision; k++) {
    // The digit at 10^-k is index exponent + k - 1 of the expansion.
    const int index = value.exponent + k - 1;
    writer.Put(index >= 0 && index < value.count ? value.digits[index] : '0');
  }
}

// d.ddddE±XX with |precision| digits after the point.
void WriteScientific(Writer& writer, const Decimal& value, int precision,
                     bool alt, bool uppercase) noexcept {
  const bool zero = value.count == 0;
  writer.Put(zero ? '0' : value.digits[0]);

  if (precision > 0) {
    writer.Put('.');
    for (int i = 1; i <= precision; i++)
      writer.Put(i < value.count ? value.digits[i] : '0');
  } else if (alt) {
    writer.Put('.');
  }

  writer.Put(uppercase ? 'E' : 'e');
  const int exponent = zero ? 0 : value.exponent - 1;
  writer.Put(exponent < 0 ? '-' : '+');
  const int magnitude = exponent < 0 ? -exponent : exponent;
  // printf pads the exponent to at least two digits.
  if (magnitude >= 100) {
    writer.Put(static_cast<char>('0' + magnitude / 100));
    writer.Put(static_cast<char>('0' + (magnitude / 10 % 10)));
    writer.Put(static_cast<char>('0' + magnitude % 10));
  } else {
    writer.Put(static_cast<char>('0' + magnitude / 10));
    writer.Put(static_cast<char>('0' + magnitude % 10));
  }
}

// %g: pick the shorter of the two layouts, then drop trailing zeros.
void WriteGeneral(Writer& writer, Decimal& value, int precision, bool alt,
                  bool uppercase) noexcept {
  int significant = precision < 0 ? 6 : (precision == 0 ? 1 : precision);
  RoundTo(value, significant);

  const bool zero = value.count == 0;
  const int scientific_exponent = zero ? 0 : value.exponent - 1;

  if (!alt) {
    // Trailing zeros are not significant in %g, so shorten the expansion and
    // let the writers pad nothing back.
    while (value.count > 0 && value.digits[value.count - 1] == '0')
      value.count--;
  }

  if (scientific_exponent >= -4 && scientific_exponent < significant) {
    int fraction = significant - 1 - scientific_exponent;
    if (!alt) {
      // Keep only the digits that survive after the point.
      const int needed = value.count - value.exponent;
      fraction = needed > 0 ? needed : 0;
    }
    WriteFixed(writer, value, fraction, alt);
  } else {
    int fraction = significant - 1;
    if (!alt)
      fraction = value.count > 0 ? value.count - 1 : 0;
    WriteScientific(writer, value, fraction, alt, uppercase);
  }
}

void WriteNonFinite(Writer& writer, bool negative, char sign, bool is_nan,
                    bool uppercase) noexcept {
  WriteSign(writer, negative, sign);
  const char* text = is_nan ? (uppercase ? "NAN" : "nan")
                            : (uppercase ? "INF" : "inf");
  writer.PutAll(text, 3);
}

}  // namespace

mem_size FormatFloatTo(char* out, mem_size capacity, f64 value, char type,
                       int precision, char sign, bool alt) noexcept {
  Writer writer(out, capacity);

  const bool uppercase = type >= 'A' && type <= 'Z';
  const char lower = uppercase ? static_cast<char>(type - 'A' + 'a') : type;

  u64 bits;
  MemCopy(&bits, &value, sizeof(bits));
  const bool negative = (bits >> 63) != 0;
  const int biased_exponent = static_cast<int>((bits >> 52) & 0x7FF);
  const u64 fraction = bits & 0xFFFFFFFFFFFFFull;

  if (biased_exponent == 0x7FF) {
    WriteNonFinite(writer, negative, sign, fraction != 0, uppercase);
    return writer.length();
  }

  u64 mantissa;
  int exponent2;
  if (biased_exponent == 0) {
    mantissa = fraction;         // zero or subnormal
    exponent2 = -1074;
  } else {
    mantissa = fraction | (1ull << 52);
    exponent2 = biased_exponent - 1075;
  }

  Decimal decimal;
  if (mantissa == 0) {
    decimal.count = 0;
    decimal.exponent = 1;  // so fixed layout emits a single leading '0'
  } else {
    ExpandExact(mantissa, exponent2, decimal);
  }

  WriteSign(writer, negative, sign);

  switch (lower) {
    case 'e':
      RoundTo(decimal, (precision < 0 ? 6 : precision) + 1);
      WriteScientific(writer, decimal, precision < 0 ? 6 : precision, alt,
                      uppercase);
      break;
    case 'g':
      WriteGeneral(writer, decimal, precision, alt, uppercase);
      break;
    case 'f':
    default: {
      const int fraction_digits = precision < 0 ? 6 : precision;
      RoundTo(decimal, decimal.exponent + fraction_digits);
      if (decimal.count == 0)
        decimal.exponent = 1;  // rounded away entirely: print 0
      WriteFixed(writer, decimal, fraction_digits, alt);
      break;
    }
  }

  return writer.length();
}

}  // namespace base
