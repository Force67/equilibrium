// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// A child of better bitset
#pragma once

#include <base/check.h>
#include <base/numeric_limits.h>
#include <base/memory/cxx_lifetime.h>
#include <base/containers/builtins_bit.h>
#include <base/containers/vector.h>

namespace base {
template <class TAllocator = base::DefaultAllocator>
class DynamicBitSet {
  using ArrayType = u64;
  using Storage = base::Vector<ArrayType, TAllocator>;

  static constexpr mem_size kWordSize = sizeof(ArrayType);

 public:

  constexpr explicit DynamicBitSet(mem_size val, mem_size bit_count) noexcept
      : bit_count_(bit_count) {

    vector_.resize(reserve_count(bit_count));
    // move full word.
    bool kNeedsMask = bit_count < kCharBit * sizeof(u64);
    auto kMask = (1ULL << (kNeedsMask ? bit_count : 0)) - 1ULL;

    SetLastMask();

    vector_.push_back(kNeedsMask ? val & kMask : val);
  }

  ~DynamicBitSet() = default;

  mem_size reserve_count(mem_size bit_count) {
    auto x = bit_count / sizeof(ArrayType);
    return x == 0 ? 1 : x;
  }

  constexpr mem_size size() const noexcept { return bit_count_; }
  constexpr mem_size storage_size() const noexcept { return vector_.size(); }

  mem_size word_count() const noexcept { return (bit_count_ + 63) / 64; }

  constexpr bool Test(mem_size pos) const noexcept { return (*this)[pos]; }

  [[nodiscard]] mem_size CountSetBits() {
    mem_size count = 0;
    for (auto i = 0; i <= storage_size(); ++i) {
      count += base::PopCount(vector_[i]);
    }
    return count;
  }

  DynamicBitSet& Set() noexcept {
    for (mem_size i = 0; i < storage_size() - 1; ++i)
      vector_[i] = base::MinMax<ArrayType>::max();
    vector_[storage_size() - 1] = last_mask_;
    return *this;
  }

  DynamicBitSet& Set(mem_size pos, bool value = true) noexcept {
    if (value == false)
      return reset(pos);
    if (bit_count_ > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      vector_[chunk] |= 1ull << shift;
      return *this;
    }
    vector_[0] |= static_cast<ArrayType>(1ull << pos);
    return *this;
  }

  [[nodiscard]] u64 to_ullong() const noexcept {
    if (bit_count_ == 0)
      return 0;
    if (bit_count_ > 64) {
      for (mem_size i = 1; i <= word_count(); ++i) {
        if (vector_[i] != 0) {
          DCHECK(true, "fail if any high-order words are nonzero");
        }
      }
    }
    return vector_[0];
  }

  // V
  DynamicBitSet& flip() noexcept {
    for (mem_size i = 0; i < word_count() - 1; ++i)
      vector_[i] = ~vector_[i];
    vector_[word_count() - 1] = static_cast<ArrayType>(
        ~static_cast<mem_size>(vector_[word_count() - 1]) & last_mask_);
    return *this;
  }

  // V
  DynamicBitSet& reset() noexcept {
    for (ArrayType& chunk : vector_)
      chunk = 0;
    return *this;
  }

  DynamicBitSet& reset(mem_size pos) noexcept {
    DCHECK(pos < bit_count_, "DynamicBitSet::Reset(): Access out of bounds");
    if (bit_count_ > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      if (chunk == word_count() - 1)
        vector_[chunk] &= ~(1ull << shift) & last_mask_;
      else
        vector_[chunk] &= ~(1ull << shift);
      return *this;
    }
    vector_[0] &= ~(1ull << pos) & last_mask_;
    return *this;
  }

  DynamicBitSet& operator=(const ArrayType val) noexcept {
    auto x = vector_.size();
    DCHECK(vector_.size() > 0, "Array is empty");
    vector_[0] = val;
    return *this;
  }

  constexpr bool operator[](mem_size pos) const {
    DCHECK(pos <= bit_count_, "DynamicBitSet::[]: Access out of bounds");
    if (bit_count_ > 64) {
      const mem_size chunk = pos / 64;
      const mem_size shift = pos % 64;
      return static_cast<bool>(vector_[chunk] >> shift);
    }
    return static_cast<bool>((vector_[0] >> pos) & 0x1);
  }

  bool operator==(const DynamicBitSet& rhs) const noexcept {
    if (rhs.size() != size())
      return false;
    return memcmp(vector_.data(), rhs.vector_.data(), storage_size()) == 0;
  }

  DynamicBitSet& operator&=(const DynamicBitSet& rhs) noexcept {
    DCHECK(bit_count_ <= 64);

    for (auto i = 0; i <= storage_size(); ++i) {
      for (auto j = 0; j <= rhs.storage_size(); ++j) {
        if (i < j)
          break;
        vector_[i] &= rhs.vector_[j] & last_mask_;
      }
    }
    return *this;
  }

  DynamicBitSet& operator|=(const DynamicBitSet& rhs) noexcept {
    for (auto i = 0; i <= storage_size(); ++i) {
      for (auto j = 0; j <= rhs.storage_size(); ++j) {
        if (i < j)
          break;
        Set(i, (*this)[i] | rhs[j]);  // we use array access operators (BitSet.[]).
      }
    }
    return *this;
  }

  DynamicBitSet& operator^=(const DynamicBitSet& rhs) noexcept {
    for (auto i = 0; i <= storage_size(); ++i) {
      for (auto j = 0; j <= rhs.storage_size(); ++j) {
        if (i < j)
          break;
        vector_[i] ^= rhs.vector_[j];
      }
    }
    return *this;
  }

 private:
  void GrowBy(mem_size words) { vector_.reserve(words); }
  void SetLastMask() { last_mask_ = ~(~1ull << ((bit_count_ - 1ull) % (64ull))); }

 private:
  Storage vector_;
  mem_size bit_count_;
  mem_size last_mask_;
};
}  // namespace base