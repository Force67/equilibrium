// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Heap allocation free version of C++11 std::function.
// base::StaticFunction stores the closure in an internal buffer instead of heap
// allocated memory. This is useful for low latency agent and thread pool
// systems. Please note that the captured values can perfom allocations, for
// example base::String.

#pragma once

#include <base/arch.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

template <class, mem_size MaxSize = 1024>
class StaticFunction;

template <class R, class... Args, mem_size MaxSize>
class StaticFunction<R(Args...), MaxSize> {
 public:
  StaticFunction() noexcept {}

  StaticFunction(std::nullptr_t) noexcept {}

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
    using f_type = typename std::decay<F>::type;
    static_assert(alignof(f_type) <= alignof(Storage), "invalid alignment");
    static_assert(sizeof(f_type) <= sizeof(Storage), "storage too small");
    new (&data_) f_type(std::forward<F>(f));
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
    StaticFunction(std::move(other)).swap(*this);
    return *this;
  }

  StaticFunction& operator=(std::nullptr_t) {
    if (manager_) {
      manager_(&data_, nullptr, Operation::Destroy);
      manager_ = nullptr;
      invoker_ = nullptr;
    }
    return *this;
  }

  template <typename F>
  StaticFunction& operator=(F&& f) {
    StaticFunction(std::forward<F>(f)).swap(*this);
    return *this;
  }

  template <typename F>
  StaticFunction& operator=(std::reference_wrapper<F> f) {
    StaticFunction(f).swap(*this);
    return *this;
  }

  void swap(StaticFunction& other) {
    std::swap(data_, other.data_);
    std::swap(manager_, other.manager_);
    std::swap(invoker_, other.invoker_);
  }

  explicit operator bool() const noexcept { return !!manager_; }

  R operator()(Args... args) {
    if (!invoker_) {
      throw std::bad_function_call();
    }
    return invoker_(&data_, base::forward<Args>(args)...);
  }

 private:
  enum class Operation { Clone, Destroy };

  using Invoker = R (*)(void*, Args&&...);
  using Manager = void (*)(void*, void*, Operation);
  using Storage =
      typename std::aligned_storage<MaxSize - sizeof(Invoker) - sizeof(Manager), 8>::type;

  template <typename F>
  static R invoke(void* data_, Args&&... args) {
    F& f = *static_cast<F*>(data_);
    return f(base::forward<Args>(args)...);
  }

  template <typename F>
  static void manage(void* dest, void* src, Operation op) {
    switch (op) {
      case Operation::Clone:
        new (dest) F(*static_cast<F*>(src));
        break;
      case Operation::Destroy:
        static_cast<F*>(dest)->~F();
        break;
    }
  }

  Storage data_;
  Invoker invoker_ = nullptr;
  Manager manager_ = nullptr;
};
}  // namespace base