// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>

namespace base {
class pointer {
 private:
  mem_size value_{0};

 public:
  constexpr pointer() = default;
  constexpr pointer(std::uintptr_t address);

  template <typename T>
  pointer(T* address);

  template <typename T, typename C>
  pointer(T C::*address);

  constexpr pointer add(mem_size count) const;
  constexpr pointer sub(mem_size count) const;

  constexpr pointer operator+(mem_size count) const;
  constexpr pointer operator-(mem_size count) const;

  constexpr pointer_diff operator-(pointer rhs) const;

  constexpr pointer& operator+=(mem_size count);
  constexpr pointer& operator-=(mem_size count);

  constexpr pointer& operator++();
  constexpr pointer& operator--();

  constexpr pointer operator++(int);
  constexpr pointer operator--(int);

  constexpr bool operator==(pointer rhs) const;
  constexpr bool operator!=(pointer rhs) const;

  constexpr bool operator<(pointer rhs) const;
  constexpr bool operator>(pointer rhs) const;

  constexpr bool operator<=(pointer rhs) const;
  constexpr bool operator>=(pointer rhs) const;

  constexpr bool operator!() const;

  constexpr explicit operator bool() const;
};

static_assert((sizeof(pointer) == sizeof(void*)) && (alignof(pointer) == alignof(void*)),
              "Hmm...");

template <typename T>
STRONG_INLINE pointer::pointer(T* address) noexcept
    : value_(reinterpret_cast<std::uintptr_t>(address)) {}

template <typename T, typename C>
STRONG_INLINE pointer::pointer(T C::*address) noexcept
    : value_(bit_cast<std::uintptr_t>(address)) {}

STRONG_INLINE constexpr pointer pointer::operator+(mem_size count) const {
  return value_ + count;
}

STRONG_INLINE constexpr pointer pointer::operator-(mem_size count) const {
  return value_ - count;
}

STRONG_INLINE constexpr pointer_diff pointer::operator-(pointer rhs) const {
  return static_cast<pointer_diff>(static_cast<pointer_diff>(value_) -
                                   static_cast<pointer_diff>(rhs.value_));
}

STRONG_INLINE constexpr pointer& pointer::operator+=(mem_size count) {
  value_ += count;
  return *this;
}

STRONG_INLINE constexpr pointer& pointer::operator-=(mem_size count) {
  value_ -= count;
  return *this;
}

STRONG_INLINE constexpr pointer& pointer::operator++() {
  ++value_;
  return *this;
}

STRONG_INLINE constexpr pointer& pointer::operator--() {
  --value_;
  return *this;
}

STRONG_INLINE constexpr pointer pointer::operator++(int) {
  pointer result = *this;
  ++value_;
  return result;
}

STRONG_INLINE constexpr pointer pointer::operator--(int) {
  pointer result = *this;
  --value_;
  return result;
}

STRONG_INLINE constexpr bool pointer::operator==(pointer rhs) const {
  return value_ == rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator!=(pointer rhs) const {
  return value_ != rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator<(pointer rhs) const {
  return value_ < rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator>(pointer rhs) const {
  return value_ > rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator<=(pointer rhs) const {
  return value_ <= rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator>=(pointer rhs) const {
  return value_ >= rhs.value_;
}

STRONG_INLINE constexpr bool pointer::operator!() const {
  return !value_;
}

STRONG_INLINE constexpr pointer::operator bool() const {
  return value_ != 0;
}
}  // namespace base