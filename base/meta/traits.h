// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Minimal type-traits, backed by compiler intrinsics where both Clang and
// GCC expose one. Keeps <type_traits> out of base.

#pragma once

namespace base {

using nullptr_t = decltype(nullptr);

template <class A, class B>
struct is_same {
  static constexpr bool value = false;
};
template <class A>
struct is_same<A, A> {
  static constexpr bool value = true;
};
template <class A, class B>
inline constexpr bool is_same_v = is_same<A, B>::value;

template <class A, class B>
concept SameAs = is_same<A, B>::value;

template <class T>
struct is_integral {
  static constexpr bool value = false;
};
#define BASE_MARK_INTEGRAL(TYPE)                 \
  template <>                                    \
  struct is_integral<TYPE> {                     \
    static constexpr bool value = true;          \
  };                                             \
  template <>                                    \
  struct is_integral<const TYPE> {               \
    static constexpr bool value = true;          \
  };                                             \
  template <>                                    \
  struct is_integral<volatile TYPE> {            \
    static constexpr bool value = true;          \
  };                                             \
  template <>                                    \
  struct is_integral<const volatile TYPE> {      \
    static constexpr bool value = true;          \
  };
BASE_MARK_INTEGRAL(bool)
BASE_MARK_INTEGRAL(char)
BASE_MARK_INTEGRAL(signed char)
BASE_MARK_INTEGRAL(unsigned char)
BASE_MARK_INTEGRAL(wchar_t)
BASE_MARK_INTEGRAL(char8_t)
BASE_MARK_INTEGRAL(char16_t)
BASE_MARK_INTEGRAL(char32_t)
BASE_MARK_INTEGRAL(short)
BASE_MARK_INTEGRAL(unsigned short)
BASE_MARK_INTEGRAL(int)
BASE_MARK_INTEGRAL(unsigned int)
BASE_MARK_INTEGRAL(long)
BASE_MARK_INTEGRAL(unsigned long)
BASE_MARK_INTEGRAL(long long)
BASE_MARK_INTEGRAL(unsigned long long)
#undef BASE_MARK_INTEGRAL

template <class T>
inline constexpr bool is_integral_v = is_integral<T>::value;

template <class T>
inline constexpr bool is_enum_v = __is_enum(T);

template <class T>
inline constexpr bool is_trivial_v = __is_trivial(T);

template <class T>
struct is_signed {
  static constexpr bool value = is_integral_v<T> && (T(-1) < T(0));
};
template <class T>
inline constexpr bool is_signed_v = is_signed<T>::value;

template <class Base, class Derived>
inline constexpr bool is_base_of_v = __is_base_of(Base, Derived);

template <class T>
struct decay {
  using type = T;
};
template <class T>
struct decay<T&> {
  using type = T;
};
template <class T>
struct decay<T&&> {
  using type = T;
};
template <class T>
struct decay<const T> {
  using type = T;
};
template <class T>
struct decay<const T&> {
  using type = T;
};
template <class T>
struct decay<volatile T> {
  using type = T;
};

template <class T>
using decay_t = typename decay<T>::type;

template <bool B, class T = void>
struct enable_if {};
template <class T>
struct enable_if<true, T> {
  using type = T;
};
template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <class E>
struct underlying_type {
  using type = __underlying_type(E);
};
template <class E>
using underlying_type_t = typename underlying_type<E>::type;

}  // namespace base
