// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// std::function substitute that stores the closure inline (no heap alloc).
// The captured values can still allocate on their own (e.g. base::String).

#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/cxx_lifetime.h>
#include <base/memory/move.h>
#include <base/meta/traits.h>

namespace base {

template <class, mem_size MaxSize = 1024>
class StaticFunction;

template <class R, class... Args, mem_size MaxSize>
class StaticFunction<R(Args...), MaxSize> {
 public:
  StaticFunction() noexcept {}

  StaticFunction(base::nullptr_t) noexcept {}

  StaticFunction(const StaticFunction& other) {
    if (other) {
      other.manager_(data_, other.data_, Operation::Clone);
      invoker_ = other.invoker_;
      manager_ = other.manager_;
    }
  }

  StaticFunction(StaticFunction&& other) { other.swap(*this); }

  template <class F>
  StaticFunction(F&& f) {
    using f_type = base::decay_t<F>;
    static_assert(alignof(f_type) <= kStorageAlign, "invalid alignment");
    static_assert(sizeof(f_type) <= kStorageSize, "storage too small");
    new (&data_) f_type(base::forward<F>(f));
    invoker_ = &invoke<f_type>;
    manager_ = &manage<f_type>;
  }

  ~StaticFunction() {
    if (manager_) {
      manager_(&data_, nullptr, Operation::Destroy);
    }
  }

  StaticFunction& operator=(const StaticFunction& other) {
    StaticFunction(other).swap(*this);
    return *this;
  }

  StaticFunction& operator=(StaticFunction&& other) {
    StaticFunction(base::move(other)).swap(*this);
    return *this;
  }

  StaticFunction& operator=(base::nullptr_t) {
    if (manager_) {
      manager_(&data_, nullptr, Operation::Destroy);
      manager_ = nullptr;
      invoker_ = nullptr;
    }
    return *this;
  }

  template <typename F>
  StaticFunction& operator=(F&& f) {
    StaticFunction(base::forward<F>(f)).swap(*this);
    return *this;
  }

  void swap(StaticFunction& other) {
    // Storage holds trivially copyable state; invoker_/manager_ know how to
    // operate on the bytes on either side.
    alignas(kStorageAlign) unsigned char tmp[kStorageSize];
    __builtin_memcpy(tmp, &data_, kStorageSize);
    __builtin_memcpy(&data_, &other.data_, kStorageSize);
    __builtin_memcpy(&other.data_, tmp, kStorageSize);
    base::swap(manager_, other.manager_);
    base::swap(invoker_, other.invoker_);
  }

  explicit operator bool() const noexcept { return !!manager_; }

  R operator()(Args... args) {
    BASE_BUGCHECK(invoker_ != nullptr, "StaticFunction invoked while empty");
    return invoker_(&data_, base::forward<Args>(args)...);
  }

 private:
  enum class Operation { Clone, Destroy };

  using Invoker = R (*)(void*, Args&&...);
  // Clone reads src, Destroy ignores it; const keeps copy construction from
  // a const source legal.
  using Manager = void (*)(void*, const void*, Operation);

  static constexpr mem_size kStorageSize =
      MaxSize - sizeof(Invoker) - sizeof(Manager);
  static constexpr mem_size kStorageAlign = 8;

  template <typename F>
  static R invoke(void* data_, Args&&... args) {
    F& f = *static_cast<F*>(data_);
    return f(base::forward<Args>(args)...);
  }

  template <typename F>
  static void manage(void* dest, const void* src, Operation op) {
    switch (op) {
      case Operation::Clone:
        new (dest) F(*static_cast<const F*>(src));
        break;
      case Operation::Destroy:
        static_cast<F*>(dest)->~F();
        break;
    }
  }

  alignas(kStorageAlign) unsigned char data_[kStorageSize];
  Invoker invoker_ = nullptr;
  Manager manager_ = nullptr;
};
}  // namespace base
