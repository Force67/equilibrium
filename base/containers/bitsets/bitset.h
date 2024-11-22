// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Simple BitSet class.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/containers/builtins_bit.h>

namespace base {
// construct a collection of N size. E.g sizeof(BitSet) for 64 bits would be 8
template <mem_size N>
class BitSet {
  using ArrayType = base::conditional_t<N <= sizeof(u32) * kCharBit, u32, u64>;

  static constexpr bool kNeedsMask = N < kCharBit * sizeof(u64);
  static constexpr auto kMask = (1ULL << (kNeedsMask ? N : 0)) - 1ULL;

 public:
  BitSet() = default;

  constexpr BitSet(unsigned long long _Val) noexcept
      : array_{static_cast<ArrayType>(kNeedsMask ? _Val & kMask : _Val)} {}

  void Reset() { array_ = {}; }

  void Set(const mem_size pos, bool toggle = true) {
    DCHECK(N >= pos, "BitSet::Set(): Invalid bit positional offset");

    auto& selected_word = array_[pos / kBitsPerWord];
    const auto new_bit = ArrayType{1} << pos % kBitsPerWord;
    toggle ? selected_word |= new_bit : selected_word &= ~new_bit;
  }

  BitSet& Flip(const mem_size pos) {
    DCHECK(N >= pos, "BitSet::Flip(): Invalid bit positional offset");
    array_[pos / kBitsPerWord] ^= ArrayType{1} << pos % kBitsPerWord;
    return *this;
  }

  [[nodiscard]] bool Test(const mem_size pos) {
    DCHECK(N >= pos, "BitSet::Test(): Invalid offset");
    return (array_[pos / kBitsPerWord] & (ArrayType{1} << pos % kBitsPerWord)) != 0;
  }

  [[nodiscard]] mem_size CountSetBits() {
    mem_size count = 0;
    for (auto i = 0; i <= kWords; ++i) {
      count += base::PopCount(array_[i]);
    }
    return count;
  }

  [[nodiscard]] u32 to_ulong() const noexcept(N <= 32) /* strengthened */ {
    constexpr bool N_zero = N == 0;
    constexpr bool N_small = N <= 32;
    constexpr bool N_large = N > 64;
    if constexpr (N_zero) {
      return 0;
    } else if constexpr (N_small) {
      return static_cast<unsigned long>(array_[0]);
    } else {
      if constexpr (N_large) {
        for (mem_size _Idx = 1; _Idx <= kWords; ++_Idx) {
          DCHECK(array_[_Idx] == 0, "fail if any high - order words are nonzero");
        }
      }

      DCHECK(array_[0] < ULONG_MAX, "Overflow");
      return static_cast<u32>(array_[0]);
    }
  }

  [[nodiscard]] u64 to_ullong() const noexcept(N <= 64) /* strengthened */ {
    constexpr bool N_zero = N == 0;
    constexpr bool N_large = N > 64;
    if constexpr (N_zero) {
      return 0;
    } else {
      if constexpr (N_large) {
        for (mem_size _Idx = 1; _Idx <= kWords; ++_Idx) {
          if (array_[_Idx] != 0) {
            DCHECK(true, "fail if any high-order words are nonzero");
          }
        }
      }

      return array_[0];
    }
  }

  BitSet& operator&=(const BitSet& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] &= rhs.array_[i];
    }
    return *this;
  }

  BitSet& operator|=(const BitSet& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] |= rhs.array_[i];
    }
    return *this;
  }

  BitSet& operator^=(const BitSet& rhs) noexcept {
    for (auto i = 0; i <= kWords; ++i) {
      array_[i] ^= rhs.array_[i];
    }
    return *this;
  }

  BitSet& operator<<=(mem_size pos) noexcept {  // shift left by pos, first by
                                                // words then by bits
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

  BitSet& operator>>=(mem_size pos) noexcept {  // shift right by pos, first by
                                                // words then by bits
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

  [[nodiscard]] bool operator==(const BitSet& rhs) const noexcept {
    return memcmp(&array_[0], &rhs.array_[0], sizeof(array_)) == 0;
  }

  [[nodiscard]] bool operator!=(const BitSet& rhs) const noexcept {
    return memcmp(&array_[0], &rhs.array_[0], sizeof(array_)) != 0;
  }

  [[nodiscard]] constexpr auto size() const noexcept { return N; }

 private:
  static constexpr pointer_diff kBitsPerWord = kCharBit * sizeof(ArrayType);
  static constexpr pointer_diff kWords =
      N == 0 ? 0 : (N - 1) / kBitsPerWord;  // NB: number of words - 1

  ArrayType array_[kWords + 1]{};
};

template <mem_size N>
[[nodiscard]] BitSet<N> operator&(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
  BitSet<N> temp = lhs;
  temp &= rhs;
  return temp;
}

template <mem_size N>
[[nodiscard]] BitSet<N> operator|(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
  BitSet<N> temp = lhs;
  temp |= rhs;
  return temp;
}

template <mem_size N>
[[nodiscard]] BitSet<N> operator^(const BitSet<N>& lhs, const BitSet<N>& rhs) noexcept {
  BitSet<N> temp = lhs;
  temp ^= rhs;
  return temp;
}
}  // namespace base