// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Simple Bitset class.
#pragma once

#include <base/arch.h>
#include <base/numeric_limits.h>
#include <intrin.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

// TODO: amd64 check.
template <class T>
[[nodiscard]] mem_size PopCount(const T value) noexcept {
  constexpr mem_size _Digits = base::MinMax<T>::digits();
  if constexpr (_Digits <= 16) {
    return static_cast<mem_size>(__popcnt16(value));
  } else if constexpr (_Digits == 32) {
    return static_cast<mem_size>(__popcnt(value));
  } else {
#ifdef _M_IX86
    return static_cast<mem_size>(__popcnt(value >> 32) +
                                 __popcnt(static_cast<unsigned int>(value)));
#else   // ^^^ _M_IX86 / !_M_IX86 vvv
    return static_cast<mem_size>(__popcnt64(value));
#endif  // _M_IX86
  }
}

// construct a collection of NBits size. E.g sizeof(BitSet) for 64 bits would be 8
template <mem_size NBits>
class Bitset {
  using ArrayType = base::conditional_t<NBits <= sizeof(u32) * kCharBit, u32, u64>;

  static constexpr bool kNeedsMask = NBits < kCharBit * sizeof(u64);
  static constexpr auto kMask = (1ULL << (kNeedsMask ? NBits : 0)) - 1ULL;

 public:
  Bitset() = default;

  constexpr Bitset(unsigned long long _Val) noexcept
      : array_{static_cast<ArrayType>(kNeedsMask ? _Val & kMask : _Val)} {}

  void Reset() { array_ = {}; }

  void Set(const mem_size pos, bool toggle = true) {
    DCHECK(NBits >= pos, "Bitset::Set(): Invalid bit positional offset");

    auto& selected_word = array_[pos / kBitsPerWord];
    const auto new_bit = ArrayType{1} << pos % kBitsPerWord;
    toggle ? selected_word |= new_bit : selected_word &= ~new_bit;
  }

  Bitset& Flip(const mem_size pos) {
    DCHECK(NBits >= pos, "Bitset::Flip(): Invalid bit positional offset");
    array_[pos / kBitsPerWord] ^= ArrayType{1} << pos % kBitsPerWord;
    return *this;
  }

  [[nodiscard]] bool Test(const mem_size pos) {
    DCHECK(NBits >= pos, "Bitset::Test(): Invalid offset");
    return (array_[pos / kBitsPerWord] & (ArrayType{1} << pos % kBitsPerWord)) != 0;
  }

  [[nodiscard]] mem_size CountSetBits() {
    mem_size count = 0;
    for (auto i = 0; i <= kWords; ++i) {
      count += base::PopCount(array_[i]);
    }
    return count;
  }

  [[nodiscard]] u32 to_ulong() const noexcept(NBits <= 32) /* strengthened */ {
    constexpr bool NBits_zero = NBits == 0;
    constexpr bool NBits_small = NBits <= 32;
    constexpr bool NBits_large = NBits > 64;
    if constexpr (NBits_zero) {
      return 0;
    } else if constexpr (NBits_small) {
      return static_cast<unsigned long>(array_[0]);
    } else {
      if constexpr (NBits_large) {
        for (mem_size _Idx = 1; _Idx <= kWords; ++_Idx) {
          if (array_[_Idx] != 0) {
            DCHECK(true, "fail if any high-order words are nonzero");
          }
        }
      }

      if (array_[0] > ULONG_MAX) {
        DCHECK(true, "Overflow");
      }

      return static_cast<unsigned long>(array_[0]);
    }
  }

  [[nodiscard]] u64 to_ullong() const noexcept(NBits <= 64) /* strengthened */ {
    constexpr bool NBits_zero = NBits == 0;
    constexpr bool NBits_large = NBits > 64;
    if constexpr (NBits_zero) {
      return 0;
    } else {
      if constexpr (NBits_large) {
        for (mem_size _Idx = 1; _Idx <= kWords; ++_Idx) {
          if (array_[_Idx] != 0) {
            DCHECK(true, "fail if any high-order words are nonzero");
          }
        }
      }

      return array_[0];
    }
  }

  Bitset& operator&=(const Bitset& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] &= rhs.array_[i];
    }
    return *this;
  }

  Bitset& operator|=(const Bitset& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] |= rhs.array_[i];
    }
    return *this;
  }

  Bitset& operator^=(const Bitset& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] ^= rhs.array_[i];
    }
    return *this;
  }

  Bitset& operator<<=(
      mem_size pos) noexcept {  // shift left by pos, first by words then by bits
    const auto word_shift = static_cast<pointer_diff>(pos / kBitsPerWord);
    if (word_shift != 0) {
      for (pointer_diff i = kWords; 0 <= i; --i) {
        array_[i] = word_shift <= i ? array_[i - word_shift] : 0;
      }
    }

    if ((pos %= kBitsPerWord) != 0) {  // 0 < pos < kBitsPerWord, shift by bits
      for (pointer_diff i = kWords; 0 < i; --i) {
        array_[i] = (array_[i] << pos) | (array_[i - 1] >> (kBitsPerWord - pos));
      }

      array_[0] <<= pos;
    }
   // _Trim();
    return *this;
  }

  Bitset& operator>>=(
      mem_size pos) noexcept {  // shift right by pos, first by words then by bits
    const auto word_shift = static_cast<pointer_diff>(pos / kBitsPerWord);
    if (word_shift != 0) {
      for (pointer_diff i = 0; i <= kWords; ++i) {
        array_[i] = word_shift <= kWords - i ? array_[i + word_shift] : 0;
      }
    }

    if ((pos %= kBitsPerWord) != 0) {  // 0 < pos < kBitsPerWord, shift by bits
      for (pointer_diff i = 0; i < kWords; ++i) {
        array_[i] = (array_[i] >> pos) | (array_[i + 1] << (kBitsPerWord - pos));
      }

      array_[kWords] >>= pos;
    }
    return *this;
  }

  [[nodiscard]] bool operator==(const Bitset& rhs) const noexcept {
    return memcmp(&array_[0], &rhs.array_[0], sizeof(array_)) == 0;
  }

  [[nodiscard]] bool operator!=(const Bitset& rhs) const noexcept {
    return memcmp(&array_[0], &rhs.array_[0], sizeof(array_)) != 0;
  }

  [[nodiscard]] constexpr auto size() const noexcept { return NBits; }

 private:
  static constexpr pointer_diff kBitsPerWord = kCharBit * sizeof(ArrayType);
  static constexpr pointer_diff kWords =
      NBits == 0 ? 0 : (NBits - 1) / kBitsPerWord;  // NB: number of words - 1

  ArrayType array_[kWords + 1]{};
};

template <mem_size NBits>
[[nodiscard]] Bitset<NBits> operator&(const Bitset<NBits>& lhs,
                                      const Bitset<NBits>& rhs) noexcept {
  Bitset<NBits> _Ans = lhs;
  _Ans &= rhs;
  return _Ans;
}

template <mem_size NBits>
_NODISCARD Bitset<NBits> operator|(const Bitset<NBits>& lhs,
                                   const Bitset<NBits>& rhs) noexcept {
  Bitset<NBits> _Ans = lhs;
  _Ans |= rhs;
  return _Ans;
}

template <mem_size NBits>
_NODISCARD Bitset<NBits> operator^(const Bitset<NBits>& lhs,
                                   const Bitset<NBits>& rhs) noexcept {
  Bitset<NBits> _Ans = lhs;
  _Ans ^= rhs;
  return _Ans;
}
}  // namespace base