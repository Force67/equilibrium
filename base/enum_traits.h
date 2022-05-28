// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Helper macros for making enum class useable...
#pragma once

namespace base {

#define BASE_IMPL_ENUM_BIT_TRAITS(Type, BaseType)         \
  inline constexpr Type operator|(Type lhs, Type rhs) {   \
    return static_cast<Type>(static_cast<BaseType>(lhs) | \
                             static_cast<BaseType>(rhs)); \
  }                                                       \
                                                          \
  inline Type operator|=(Type& lhs, Type rhs) {           \
    auto new_val = reinterpret_cast<BaseType*>(&lhs);     \
    *new_val |= static_cast<BaseType>(rhs);               \
    return static_cast<Type>(*new_val);                   \
  }                                                       \
                                                          \
  inline constexpr bool operator&(Type lhs, Type rhs) {   \
    return static_cast<bool>(static_cast<BaseType>(lhs) & \
                             static_cast<BaseType>(rhs)); \
  }  // namespace base
}  // namespace base
