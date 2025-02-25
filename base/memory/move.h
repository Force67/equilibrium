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

template <typename T>
struct remove_pointer {
  using type = T;
};

template <typename T>
struct remove_pointer<T*> {
  using type = T;
};

template <typename T>
struct remove_pointer<T* const> {
  using type = T;
};

template <typename T>
struct is_pointer {
  static constexpr bool value = false;
};

template <typename T>
struct is_pointer<T*> {
  static constexpr bool value = true;
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
constexpr typename remove_reference<T>::type&& move(T&& x) noexcept {
  return static_cast<typename remove_reference<T>::type&&>(x);
}

template <class T>
[[nodiscard]] constexpr T* AddressOf(T& v) noexcept {
  return __builtin_addressof(v);
}

#define BASE_NOCOPYMOVE(class_name)                  \
  class_name(class_name&&) = delete;                 \
  class_name(const class_name&) = delete;            \
  class_name& operator=(const class_name&) = delete; \
  class_name& operator=(class_name&&) = delete;

#define BASE_NOMOVE(class_name)            \
  class_name(const class_name&&) = delete; \
  class_name& operator=(class_name&&) = delete;

#define BASE_NOCOPY(class_name)           \
  class_name(const class_name&) = delete; \
  class_name& operator=(const class_name&) = delete;
}  // namespace base
