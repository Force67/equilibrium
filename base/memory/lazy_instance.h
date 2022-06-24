// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>

namespace base {
// prefer this class over unneeded unique_pointers if your only goal is to delay
// creation. use with composition.
template <typename T>
requires(!base::IsTrivial<T>) class LazyInstance {
 public:
  constexpr LazyInstance() = default;
  constexpr ~LazyInstance() { as_obj().~T(); }

  BASE_NOCOPYMOVE(LazyInstance);

  void Make() {
    DCHECK(!exists_);
    exists_ = true;
    new (&storage_[0]) T();
  }

  template <typename... TArgs>
  void Make(TArgs&&... args) {
    DCHECK(!exists_);
    exists_ = true;
    new (&storage_[0]) T(base::forward(args...));
  }

  // if the instance has been made yet
  bool constructed() const noexcept { return exists_; }

  T* operator->() const noexcept {
    DCHECK(exists_);
    return &as_obj();
  }

  T& operator*() const noexcept {
    DCHECK(exists_);
    return as_obj();
  }

 private:
  T& as_obj() noexcept { return *reinterpret_cast<T*>(&storage_); }

 private:
  // we cannot assume there wouldnd be a valid object thatd be 0 in data
  // so we rely on the extra boolean to be absolutely sure.
  bool exists_{false};

  // we dont declare this as the master alignment in hopes of getting a better total
  // alignment with the boolean
  u8 /* alignas(T)*/ storage_[sizeof(T)]{0};
};
}  // namespace base
