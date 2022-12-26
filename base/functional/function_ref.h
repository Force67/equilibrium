// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// Implementation of a simplified function_view
// which supports generic callable objects (but doesn’t have any syntactic sugar for
// member functions). We’ll also only implement the constructor and operator()
// Inspired by:
// https://vittorioromeo.info/index/blog/passing_functions_to_functions.html

#include <xstddef>
#include <memory/move.h>

namespace base {

template <typename Signature>
class FunctionRef;

template <typename TReturn, typename... Args>
class FunctionRef<TReturn(Args...)> final {
 public:
  using signature_type = TReturn(void*, Args...);

  template <typename Functor>
  FunctionRef(Functor&& functor) : functor_((void*)std::addressof(functor)) {
    // An explicit (void*) cast is being used to drop const qualifiers and accept
    // function pointers. reinterpret_cast and std::add_pointer_t are being used to
    // “rebuild” the original type-erased pointer. Using static_cast would not
    // support function pointers. Using T* would be ill-formed when T is an lvalue
    // reference. (Thanks Yakk and T.C.!)
    // base::forward is being used in an unusual context here, as TArgs... is not a
    // deduced argument pack, but it is required to maintain the correct value
    // categories
    erased_function_ = [](void* ptr, Args... args) -> decltype(auto) {
      return (*static_cast<Functor*>(ptr))(base::forward<Args>(args)...);
    };
  }

  TReturn operator()(Args... args) noexcept {
    return erased_function_(functor_, base::forward<Args>(args)...);
  }

 private:
  void* functor_;
  signature_type* erased_function_;
};
}  // namespace base