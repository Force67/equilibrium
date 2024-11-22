// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// UniquePtr implementation that aims to be more safe than its std counterpart,
// see https://www.reddit.com/r/cpp/comments/pkru4h/safer_usage_of_c_in_chrome/
// for more info
#pragma once

#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/deleter.h>
#include <base/memory/cxx_lifetime.h>

#include <type_traits>

namespace base {
// unique_ptr replacement class, but you are:
// - forced to use MakeUnique, so you cannot use it with a foreign pointer
// - no get, so you cannot leak the object.
template <typename T, class TDeleter = DefaultDeleter<T>>
// disable using void, required to allow array specialization
  requires(!base::ISSame<T, void>)
class UniquePointer {
 public:
  using TType = base::remove_extent_t<T>;

 public:
  // we forbid direct useage of this in order to force the user to use the
  // MakeUnique function which prevents dangleing references
  constexpr UniquePointer(TType* pointer) noexcept : pointer_(pointer) {}

 public:
  // create empty pointer
  constexpr UniquePointer() noexcept : pointer_(nullptr) {}

  // move constructor exactly the same type
  constexpr UniquePointer(UniquePointer&& rhs) noexcept : pointer_(rhs.pointer_) {
    rhs.pointer_ = nullptr;
  }

  // move constructor from base type
  // e.g when assigning UniquePtr<Base> b = move(UniquePtr<Super>....
  template <typename U>
    requires(std::is_base_of_v<T, U>)
  constexpr UniquePointer(UniquePointer<U>&& rhs) noexcept
      : pointer_(rhs.Get_UseOnlyIfYouKnowWhatYouareDoing()) {
    rhs.ResetUnchecked_UseOnlyIfYouKnowWhatYouareDoing();
  }

  ~UniquePointer() {
    if (pointer_)
      Free();
  }
  // must be used for construction
  template <typename TT, typename... TArgs>
  friend UniquePointer<TT> MakeUnique(TArgs&&... args)
    requires(!base::IsArray<TT>);
  template <typename TT>
  friend UniquePointer<TT> MakeUnique(mem_size count)
    requires(base::IsArray<TT>);

  // you have to request move
  BASE_NOCOPY(UniquePointer)

  inline UniquePointer& operator=(UniquePointer&& rhs) noexcept
  /*TODO(Vince): requires(is_move_assignable_v<T>)*/ {
    if (this != base::AddressOf(rhs)) {
      // steal & invalidate right side.
      pointer_ = rhs.pointer_;
      rhs.pointer_ = nullptr;
    }
    return *this;
  }

#if 0
  template <typename U>
  inline UniquePointer<U>& operator=(UniquePointer&& rhs) noexcept
      requires(std::convertible_to<T, U>)
  /*TODO(Vince): requires(is_move_assignable_v<T>)*/ {
    if (this != base::AddressOf(rhs)) {
      // steal & invalidate right side.
      pointer_ = rhs.pointer_;
      rhs.pointer_ = nullptr;
    }
    return *this;
  }
#endif

  // Reset method to replace the managed object with a new one.
  template <typename U>
  void Reset(UniquePointer<U>&& newObject) noexcept {
    if (pointer_ != nullptr) {
      Free();
    }
    pointer_ = newObject.Get_UseOnlyIfYouKnowWhatYouareDoing();
    newObject.ResetUnchecked_UseOnlyIfYouKnowWhatYouareDoing();
  }

  // Reset method to replace the managed object with a new one from a raw
  // pointer.
  void Reset(TType* newPointer = nullptr) noexcept {
    if (pointer_) {
      Free();
    }
    pointer_ = newPointer;
  }

  // release all memory owned by this pointer
  void Free() {
    DCHECK(pointer_);
    // restore former type info for array types so it decays to delete[] instead
    // of delete
    TDeleter::Delete(reinterpret_cast<T*>(pointer_));
    // make sure the pointer was invalidated
    pointer_ = nullptr;
  }

  TType* operator->() const noexcept {
    DCHECK(pointer_);
    return pointer_;
  }

  TType& operator*() const noexcept {
    DCHECK(pointer_);
    return *pointer_;
  }

  // array access
  TType& operator[](mem_size index)
    requires(base::IsArray<T>)
  {
    // DCHECK(index < (sizeof(T) / sizeof(pointer_[0])));
    return pointer_[index];
  }

  bool empty() const noexcept { return pointer_ == nullptr; }
  operator bool() const noexcept { return pointer_ != nullptr; }

  TType* Get_UseOnlyIfYouKnowWhatYouareDoing() const { return pointer_; }
  void ResetUnchecked_UseOnlyIfYouKnowWhatYouareDoing() { pointer_ = nullptr; }

 private:
  TType* pointer_;
};

static_assert(sizeof(UniquePointer<u8>) == sizeof(void*), "UniquePtr is misaligned");

template <typename T, typename... TArgs>
[[nodiscard]] UniquePointer<T> MakeUnique(TArgs&&... args)
  requires(!base::IsArray<T>)
{
  return UniquePointer<T>(new T(base::forward<TArgs>(args)...));
}

template <typename T>
[[nodiscard]] UniquePointer<T> MakeUnique(const mem_size count)
  requires(base::IsArray<T>)
{
  using TElem = base::remove_extent_t<T>;
  return UniquePointer<T>(new TElem[count]);
}
}  // namespace base
