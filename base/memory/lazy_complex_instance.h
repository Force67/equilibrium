// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>

namespace base {
// prefer this class over unneeded unique_pointers if your only goal is to delay
// creation. use with composition. This is a no overhead version of the same class,
// for which you need to implement a _constructed_ method. Think _carefully_ about
// what kind of members you check for construction
template <typename T>
requires(!base::IsTrivial<T>) class LazyComplexInstance {
 public:
  constexpr LazyComplexInstance() = default;

  constexpr ~LazyComplexInstance() {
    if (!T::constructed())
      as_obj().~T();
  }

  BASE_NOCOPYMOVE(LazyComplexInstance)

  void Make() {
    DCHECK(!T::constructed());
    new (&storage_[0]) T();
  }

  template <typename... TArgs>
  void Make(TArgs&&... args) {
    DCHECK(!T::constructed());
    new (&storage_[0]) T(base::forward<TArgs>(args)...);
  }

  // if the instance has been made yet
  operator bool() const noexcept { return T::construced(); }

  T* operator->() noexcept {
    DCHECK(T::constructed());
    return &as_obj();
  }

  T& operator*() const noexcept {
    DCHECK(T::constructed());
    return as_obj();
  }

 private:
  T& as_obj() noexcept { return *reinterpret_cast<T*>(&storage_); }

 private:
  // we dont declare this as the master alignment in hopes of getting a better total
  // alignment with the boolean
  u8 alignas(T) storage_[sizeof(T)]{0};
};
}  // namespace base
