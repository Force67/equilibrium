// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Source: https://github.com/MrElectrify/better_bitset
#pragma once

#include <base/check.h>
#include <base/numeric_limits.h>
#include <base/memory/cxx_lifetime.h>
#include <base/containers/builtins_bit.h>

namespace base {
template <mem_size N>
  requires(N > 0)
class BetterBitSet {
  using ArrayType = base::conditional_t<
      (N > 32),
      u64,
      base::conditional_t<(N > 16), u32, base::conditional_t<(N > 8), u16, u8>>>;

  static constexpr mem_size kLastMask = ~(~1ull << ((N - 1ull) % (64ull)));
  static constexpr mem_size kWords = (N + 63) / 64;
  static constexpr bool kNeedsMask = N < kCharBit * sizeof(u64);
  static constexpr auto kMask = (1ULL << (kNeedsMask ? N : 0)) - 1ULL;

  using Storage = ArrayType[kWords];

 public:
  constexpr BetterBitSet() noexcept : array_() {}
  constexpr BetterBitSet(unsigned long long val) noexcept
      : array_{static_cast<ArrayType>(kNeedsMask ? val & kMask : val)} {}

  [[nodiscard]] mem_size CountSetBits() {
    mem_size count = 0;
    for (auto i = 0; i < kWords; ++i) {
      count += base::PopCount(array_[i]);
    }
    return count;
  }

  constexpr bool Test(mem_size pos) const noexcept { return (*this)[pos]; }

  constexpr mem_size size() const noexcept { return N; }

  BetterBitSet& Set() noexcept {
    for (mem_size i = 0; i < kWords - 1; ++i)
      array_[i] = base::MinMax<ArrayType>::max();
    array_[kWords - 1] = kLastMask;
    return *this;
  }

  BetterBitSet& Set(mem_size pos, bool value = true) noexcept {
    DCHECK(pos < N);
    if (value == false)
      return reset(pos);
    if constexpr (N > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      array_[chunk] |= 1ull << shift;
      return *this;
    }
    array_[0] |= static_cast<ArrayType>(1ull << pos);
    return *this;
  }

  [[nodiscard]] auto to_number() const noexcept(N <= 32) /* strengthened */ {
    if constexpr (N == 0) {
      return 0;
    } else if constexpr (N <= 8) {
      return static_cast<u8>(array_[0]);
    } else if constexpr (N <= 16) {
      return static_cast<u16>(array_[0]);
    } else if constexpr (N <= 32) {
      return static_cast<u32>(array_[0]);
    } else {
      if constexpr (N > 64) {
        for (mem_size _Idx = 1; _Idx < kWords; ++_Idx) {
          DCHECK(array_[_Idx] != 0, "fail if any high-order words are nonzero");
        }
      }

      DCHECK(array_[0] < ULONG_MAX, "Overflow");
      return static_cast<u32>(array_[0]);
    }
  }

  // V
  BetterBitSet& flip() noexcept {
    for (mem_size i = 0; i < kWords - 1; ++i)
      array_[i] = ~array_[i];
    array_[kWords - 1] =
        static_cast<ArrayType>(~static_cast<mem_size>(array_[kWords - 1]) & kLastMask);
    return *this;
  }

  // V
  BetterBitSet& reset() noexcept {
    for (ArrayType& chunk : array_)
      chunk = 0;
    return *this;
  }

  BetterBitSet& reset(mem_size pos) noexcept {
    DCHECK(pos < N, "BetterBitSet::Reset(): Access out of bounds");
    if constexpr (N > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      if (chunk == kWords - 1)
        array_[chunk] &= ~(1ull << shift) & kLastMask;
      else
        array_[chunk] &= ~(1ull << shift);
      return *this;
    }
    array_[0] &= ~(1ull << pos) & kLastMask;
    return *this;
  }

  BetterBitSet& operator=(const ArrayType value) noexcept {
    array_[0] = value;
    return *this;
  }

  // requires guard!!!!!!
  BetterBitSet& operator=(const BetterBitSet& rhs) noexcept {
    memcpy(&array_[0], &rhs.array_[0], sizeof(Storage));
    return *this;
  }

  [[nodiscard]] CONSTEXPR_ND bool operator[](mem_size pos) const {
    DCHECK(pos < N, "BetterBitSet::[]: Access out of bounds");
    if constexpr (N > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      return static_cast<bool>(array_[chunk] >> shift);
    }
    return static_cast<bool>((array_[0] >> pos) & 0x1);
  }

  constexpr bool operator==(const BetterBitSet<N>& rhs) const noexcept {
    return memcmp(&array_[0], &rhs.array_[0], sizeof(array_)) == 0;
  }

  BetterBitSet& operator&=(const BetterBitSet& rhs) noexcept
    requires(N <= 64)
  {
    for (auto i = 0; i < kWords; ++i) {
      array_[i] &= rhs.array_[i] & kLastMask;
    }
    return *this;
  }

  BetterBitSet& operator|=(const BetterBitSet& rhs) noexcept {
    for (auto i = 0; i < kWords; ++i) {
      Set(i, this->operator[](i) | rhs[i]);  // we use array access operators (BitSet.[]).
    }

    return *this;
  }

  BetterBitSet& operator^=(const BetterBitSet& rhs) noexcept {
    for (auto i = 0; i < kWords; ++i) {
      array_[i] ^= rhs.array_[i];
    }
    return *this;
  }

 private:
  Storage array_{};
};

template <mem_size N>
[[nodiscard]] BetterBitSet<N> operator&(const BetterBitSet<N>& lhs,
                                        const BetterBitSet<N>& rhs) noexcept {
  auto temp = lhs;
  temp &= rhs;
  return temp;
}

template <mem_size N>
[[nodiscard]] BetterBitSet<N> operator|(const BetterBitSet<N>& lhs,
                                        const BetterBitSet<N>& rhs) noexcept {
  auto temp = lhs;
  temp |= rhs;
  return temp;
}

template <mem_size N>
[[nodiscard]] BetterBitSet<N> operator^(const BetterBitSet<N>& lhs,
                                        const BetterBitSet<N>& rhs) noexcept {
  auto temp = lhs;
  temp ^= rhs;
  return temp;
}
}  // namespace base