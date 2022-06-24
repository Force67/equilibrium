// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/deleter.h>

namespace base {
enum class MakeLater {};

// this class by design does *not* expose any of the following:
// - .get() as this can lead to use after free
// - .free() as this can lead to dangeling references
template <typename T, class TDeleter = base::DefaultDeleter<T>>
class DistinctPointer {
 public:
  // default ctor, pointer is uninitialized
  template <typename TT>
  constexpr DistinctPointer(TT) requires(std::is_same_v<TT, MakeLater>)
      : pointer_(nullptr){};

  template <typename... TArgs>
  constexpr DistinctPointer(TArgs&&... args) requires(
      !std::is_same_v<std::common_type<TArgs...>, MakeLater>)
      : pointer_(new T(base::forward<TArgs>(args)...)) {}

  constexpr ~DistinctPointer() { TDeleter::Delete(pointer_); }

  // for delayed initialization
  template <typename... TArgs>
  void Make(TArgs&&... args) {
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

 private:
  T* pointer_{nullptr};
};
}  // namespace base