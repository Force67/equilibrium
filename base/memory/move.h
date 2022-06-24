// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {
template <typename T>
struct remove_reference {
  using type = T;
};
template <typename T>
struct remove_reference<T&> {
  using type = T;
};
template <typename T>
struct remove_reference<T&&> {
  using type = T;
};

// typename remove_reference<T>::type&
template <typename T>
constexpr T&& forward(T& x) noexcept {
  return static_cast<T&&>(x);
}

// typename remove_reference<T>::type&&
template <typename T>
constexpr T&& forward(T&& x) noexcept  // requires(is_lvalue_reference<T>
{
  // static_assert(!is_lvalue_reference<T>::value, "forward T isn't lvalue
  // reference");
  return static_cast<T&&>(x);
}

template <typename T>
constexpr remove_reference<T>::type&& move(T&& x) noexcept {
  return static_cast<remove_reference<T>::type&&>(x);
}
}  // namespace base