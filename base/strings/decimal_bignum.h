// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Internal to base's number conversions: the fixed-capacity big integer that
// exact binary-to-decimal (float_format.cc) and decimal-to-binary
// (number_parse.cc) both need. Not a general bignum and not public -- it
// carries exactly the operations those two use and no more.
#pragma once

#include <base/arch.h>

namespace base {
namespace numeric_detail {

// Limbs are 32 bits so every multiply lands in a u64 and every divide is a
// u64 by u32, with no 128-bit type -- which MSVC spells differently.
constexpr int kLimbBits = 32;

// Sized for the widest intermediate either direction produces. Formatting
// needs m * 5^1074, which is 2548 bits. Parsing needs the numerator shifted
// far enough left that the quotient still has 54 significant bits after
// dividing by a power of ten, which runs to roughly 6700 bits.
constexpr int kMaxLimbs = 224;  // 7168 bits

class BigUInt {
 public:
  void SetU64(u64 value) noexcept {
    used_ = 0;
    while (value) {
      limb_[used_++] = static_cast<u32>(value & 0xFFFFFFFFu);
      value >>= 32;
    }
  }

  bool IsZero() const noexcept { return used_ == 0; }

  // Multiplies by |multiplier| and adds |addend|, which is how decimal digits
  // are accumulated one at a time. Saturates rather than overflowing the
  // fixed capacity; the callers bound their input so it cannot be reached.
  void MulAddSmall(u32 multiplier, u32 addend) noexcept {
    u64 carry = addend;
    for (int i = 0; i < used_; i++) {
      const u64 product = static_cast<u64>(limb_[i]) * multiplier + carry;
      limb_[i] = static_cast<u32>(product & 0xFFFFFFFFu);
      carry = product >> 32;
    }
    while (carry && used_ < kMaxLimbs) {
      limb_[used_++] = static_cast<u32>(carry & 0xFFFFFFFFu);
      carry >>= 32;
    }
  }

  void MulSmall(u32 multiplier) noexcept { MulAddSmall(multiplier, 0); }

  void ShiftLeft(int bits) noexcept {
    if (IsZero() || bits <= 0)
      return;
    const int whole = bits / kLimbBits;
    const int part = bits % kLimbBits;
    if (part) {
      u32 carry = 0;
      for (int i = 0; i < used_; i++) {
        const u32 next = limb_[i] >> (kLimbBits - part);
        limb_[i] = (limb_[i] << part) | carry;
        carry = next;
      }
      if (carry && used_ < kMaxLimbs)
        limb_[used_++] = carry;
    }
    if (whole) {
      for (int i = used_ - 1; i >= 0; i--) {
        if (i + whole < kMaxLimbs)
          limb_[i + whole] = limb_[i];
      }
      for (int i = 0; i < whole && i < kMaxLimbs; i++)
        limb_[i] = 0;
      used_ += whole;
      if (used_ > kMaxLimbs)
        used_ = kMaxLimbs;
    }
  }

  // Divides in place and returns the remainder.
  u32 DivModSmall(u32 divisor) noexcept {
    u64 remainder = 0;
    for (int i = used_ - 1; i >= 0; i--) {
      const u64 current = (remainder << 32) | limb_[i];
      limb_[i] = static_cast<u32>(current / divisor);
      remainder = current % divisor;
    }
    Trim();
    return static_cast<u32>(remainder);
  }

  // Position of the highest set bit, counting from one; zero for a zero value.
  int BitLength() const noexcept {
    if (used_ == 0)
      return 0;
    const u32 top = limb_[used_ - 1];
    int bits = 0;
    for (u32 v = top; v; v >>= 1)
      bits++;
    return (used_ - 1) * kLimbBits + bits;
  }

  bool BitAt(int index) const noexcept {
    if (index < 0 || index >= used_ * kLimbBits)
      return false;
    return (limb_[index / kLimbBits] >> (index % kLimbBits)) & 1u;
  }

  // Whether any bit strictly below |index| is set. Together with the bit at
  // |index| this is the round-and-sticky pair that rounding needs.
  bool AnyBitBelow(int index) const noexcept {
    if (index <= 0)
      return false;
    const int limit = index < used_ * kLimbBits ? index : used_ * kLimbBits;
    for (int i = 0; i < limit; i++) {
      if (BitAt(i))
        return true;
    }
    return false;
  }

  // The |count| bits below the most significant one, as a u64, with
  // |dropped_nonzero| reporting whether anything below them was set. |count|
  // must not exceed 64.
  u64 HighBits(int count, bool& dropped_nonzero) const noexcept {
    const int length = BitLength();
    u64 value = 0;
    const int lowest = length - count;
    for (int i = length - 1; i >= 0 && i >= lowest; i--)
      value = (value << 1) | (BitAt(i) ? 1u : 0u);

    dropped_nonzero = false;
    for (int i = 0; i < lowest; i++) {
      if (BitAt(i)) {
        dropped_nonzero = true;
        break;
      }
    }
    return value;
  }

 private:
  void Trim() noexcept {
    while (used_ > 0 && limb_[used_ - 1] == 0)
      used_--;
  }

  u32 limb_[kMaxLimbs] = {};
  int used_ = 0;
};

}  // namespace numeric_detail
}  // namespace base
