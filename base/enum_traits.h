// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Helper macros for making enum class useable...
#pragma once

namespace base {

// https://github.com/0x1F9F1/mem/blob/db0289a50da77101c4e827b92b39f06ba2e90f76/include/mem/bitwise_enum.h
#define BASE_IMPL_ENUM_BIT_TRAITS(Type, BaseType)                                      \
  inline constexpr Type operator|(Type lhs, Type rhs) noexcept {                       \
    return static_cast<Type>(static_cast<BaseType>(lhs) | static_cast<BaseType>(rhs)); \
  }                                                                                    \
                                                                                       \
  inline constexpr Type& operator|=(Type& lhs, Type rhs) noexcept {                    \
    return lhs = lhs | rhs;                                                            \
  }                                                                                    \
                                                                                       \
  inline constexpr bool operator&(Type lhs, Type rhs) noexcept {                       \
    return static_cast<bool>(static_cast<BaseType>(lhs) & static_cast<BaseType>(rhs)); \
  }                                                                                    \
                                                                                       \
  inline constexpr Type operator~(Type a) noexcept {                                   \
    return static_cast<Type>(~static_cast<BaseType>(a));                               \
  }                                                                                    \
                                                                                       \
  inline constexpr Type operator^(Type a, Type b) noexcept {                           \
    return static_cast<Type>(static_cast<BaseType>(a) ^ static_cast<BaseType>(b));     \
  }                                                                                    \
                                                                                       \
  inline constexpr Type& operator^=(Type& a, Type b) noexcept {                        \
    return a = a ^ b;                                                                  \
  }
}  // namespace base
