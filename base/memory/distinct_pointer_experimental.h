// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/deleter.h>

namespace base {
enum class MakeNow {};
enum class MakeLater {};

// this class by design does *not* expose any of the following:
// - .get() as this can lead to use after free
// - .free() as this can lead to dangeling references
template <typename T,
          class TConstructionPolicy = MakeNow,
          class TDeleter = base::DefaultDeleter<T>>
class DistinctPointer {
 public:
  // default ctor, pointer is uninitialized
  constexpr DistinctPointer() requires(base::ISSame<TConstructionPolicy, MakeLater>)
      : pointer_(nullptr){};

  constexpr DistinctPointer() requires(base::ISSame<TConstructionPolicy, MakeNow>)
      : pointer_(new T()) {}

  template <typename... TArgs>
  constexpr DistinctPointer(TArgs&&... args) requires(
      base::ISSame<TConstructionPolicy, MakeNow>)
      : pointer_(new T(base::forward<TArgs>(args)...)) {}

  constexpr ~DistinctPointer() {
    if (pointer_)
      TDeleter::Delete(pointer_);
  }

  BASE_NOCOPYMOVE(DistinctPointer);

  // delay without params
  void Make() requires(base::ISSame<TConstructionPolicy, MakeLater>) {
    DCHECK(!pointer_);
    pointer_ = new T();
  }

  // for delayed initialization
  template <typename... TArgs>
  void Make(TArgs&&... args) requires(base::ISSame<TConstructionPolicy, MakeLater>) {
    DCHECK(!pointer_);
    pointer_ = new T(base::forward<TArgs>(args)...);
  }

  T* operator->() const {
    DCHECK(pointer_);
    return pointer_;
  }

  T& operator*() const {
    DCHECK(pointer_);
    return *pointer_;
  }

  constexpr bool empty() const { return pointer_ == nullptr; }
  operator bool() const noexcept { return pointer_ != nullptr; }

 private:
  T* pointer_{nullptr};
};
}  // namespace base