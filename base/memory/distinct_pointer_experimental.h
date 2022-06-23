// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/memory/deleter.h>

namespace base {

// this class by design does *not* expose any of the following:
// - .get() as this can lead to use after free
// - .free() as this can lead to dangeling references
template <typename T, class TDeleter = DefaultDeleter<T>>
class DistinctPointer {
 public:
  // default ctor, pointer is uninitialized
  DistinctPointer() : pointer_(nullptr){};

  // TODO: must forward args.
  template <typename... TArgs>
  // disallow type confusion for classes that have an
  // parameterless default constructor
  // e.g   base::DistinctPointer<ComplexObjectNoCtor> var();
  // EXPECT_TRUE(var.empty());
  // would never work
  DistinctPointer(TArgs&&... args) requires(sizeof...(TArgs) > 0)
      : pointer_(new T(args...)) {}

  ~DistinctPointer() { TDeleter::Delete(pointer_); }

  // for delayed initialization

  // TODO: must forward
  template <typename... TArgs>
  void Make(TArgs&&... args) {
    DCHECK(!pointer_);
    pointer_ = new T(args...);
  }

  T* operator->() const {
    DCHECK(pointer_);
    return pointer_;
  }

  T& operator*() const {
    DCHECK(pointer_);
    return *pointer_;
  }

  bool empty() const { return pointer_ == nullptr; }

 private:
  T* pointer_{nullptr};
};
}  // namespace base