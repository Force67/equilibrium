// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>
#include <base/memory/deleter.h>

namespace base {
// we dont allow creation from a raw pointer
// OwnedPointer would be a better name tbh..
// not a fan of allowing construction from another object, because it can lead to UAF

template <typename T, class TDeleter = DefaultDeleter<T>>
class UniquePointer {
 public
  explicit constexpr UniquePointer(T* pointer) noexcept : pointer_(pointer) {}
  ~UniquePointer() { Free(); }

  // release all memory owned by this pointer
  void Free() {
    DCHECK(pointer_);
    TDeleter::Delete(pointer_);
  }

  // this will ensure dangleing references// not preffered to use...
  // https://www.reddit.com/r/cpp/comments/pkru4h/safer_usage_of_c_in_chrome/
  // T* get() const noexcept { return pointer_; }
  T* operator->() const noexcept { return pointer_; }
  T& operator*() const  // Not noexcept, because the pointer may be NULL.
  {
    return *pointer_;
  }

 private:
  T* pointer_;
};

template <typename T, typename... TArgs>
UniquePointer<T> MakeUnique(TArgs&&... args) {
  T* object = new T(static_cast<TArgs&&>(args)...);
  return UniquePointer<T>(object);
}
}  // namespace base