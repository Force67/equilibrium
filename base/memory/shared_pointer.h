// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::SharedPointer: shared ownership with a thread-safe reference count,
// the std::shared_ptr counterpart. Constructed only through MakeShared, which
// places the object and its count in one allocation, so it never adopts a
// foreign pointer.
#pragma once

#include <base/atomic.h>
#include <base/check.h>
#include <base/memory/move.h>
#include <base/meta/traits.h>

namespace base {

namespace detail {
struct SharedControl {
  Atomic<mem_size> refs{1};
  virtual void Destroy() = 0;

 protected:
  ~SharedControl() = default;
};

template <typename T>
struct SharedBlock final : SharedControl {
  template <typename... TArgs>
  explicit SharedBlock(TArgs&&... args) : value(base::forward<TArgs>(args)...) {}
  void Destroy() override { delete this; }
  T value;
};
}  // namespace detail

template <typename T>
class SharedPointer {
 public:
  constexpr SharedPointer() noexcept = default;
  constexpr SharedPointer(base::nullptr_t) noexcept {}

  SharedPointer(const SharedPointer& other) noexcept
      : pointer_(other.pointer_), control_(other.control_) {
    Retain();
  }
  SharedPointer(SharedPointer&& other) noexcept
      : pointer_(other.pointer_), control_(other.control_) {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  // From a pointer to a derived or less const-qualified type.
  template <typename U>
    requires(base::ConvertibleTo<U*, T*>)
  SharedPointer(const SharedPointer<U>& other) noexcept
      : pointer_(other.pointer_), control_(other.control_) {
    Retain();
  }
  template <typename U>
    requires(base::ConvertibleTo<U*, T*>)
  SharedPointer(SharedPointer<U>&& other) noexcept
      : pointer_(other.pointer_), control_(other.control_) {
    other.pointer_ = nullptr;
    other.control_ = nullptr;
  }

  ~SharedPointer() { Release(); }

  SharedPointer& operator=(const SharedPointer& other) noexcept {
    SharedPointer(other).Swap(*this);
    return *this;
  }
  SharedPointer& operator=(SharedPointer&& other) noexcept {
    SharedPointer(base::move(other)).Swap(*this);
    return *this;
  }
  SharedPointer& operator=(base::nullptr_t) noexcept {
    Reset();
    return *this;
  }

  void Reset() noexcept {
    Release();
    pointer_ = nullptr;
    control_ = nullptr;
  }

  T* get() const noexcept { return pointer_; }
  T& operator*() const noexcept {
    BASE_DCHECK(pointer_);
    return *pointer_;
  }
  T* operator->() const noexcept {
    BASE_DCHECK(pointer_);
    return pointer_;
  }
  explicit operator bool() const noexcept { return pointer_ != nullptr; }

  // Owners right now; a moment later other threads may have changed it.
  mem_size use_count() const noexcept {
    return control_ ? control_->refs.load(base::memory_order_relaxed) : 0;
  }

  template <typename U>
  bool operator==(const SharedPointer<U>& other) const noexcept {
    return pointer_ == other.pointer_;
  }
  bool operator==(base::nullptr_t) const noexcept { return pointer_ == nullptr; }

  void Swap(SharedPointer& other) noexcept {
    T* p = pointer_;
    pointer_ = other.pointer_;
    other.pointer_ = p;
    detail::SharedControl* c = control_;
    control_ = other.control_;
    other.control_ = c;
  }

 private:
  template <typename U>
  friend class SharedPointer;
  template <typename U, typename... TArgs>
  friend SharedPointer<U> MakeShared(TArgs&&... args);

  SharedPointer(T* pointer, detail::SharedControl* control) noexcept
      : pointer_(pointer), control_(control) {}

  void Retain() noexcept {
    if (control_)
      control_->refs.fetch_add(1, base::memory_order_relaxed);
  }
  void Release() noexcept {
    if (control_ &&
        control_->refs.fetch_sub(1, base::memory_order_acq_rel) == 1)
      control_->Destroy();
  }

  T* pointer_ = nullptr;
  detail::SharedControl* control_ = nullptr;
};

template <typename T, typename... TArgs>
SharedPointer<T> MakeShared(TArgs&&... args) {
  using Stored = base::remove_const_t<T>;
  auto* block = new detail::SharedBlock<Stored>(base::forward<TArgs>(args)...);
  return SharedPointer<T>(&block->value, block);
}

}  // namespace base
