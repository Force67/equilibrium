// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Non owning, non allocating reference to a callable. Intended for in-call
// use such as function parameters. The referenced callable must outlive the
// FunctionRef, so never store one as a member; use Function or StaticFunction
// for that.
#pragma once

#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

template <typename Signature>
class FunctionRef;

template <typename TReturn, typename... TArgs>
class FunctionRef<TReturn(TArgs...)> final {
 public:
  FunctionRef() = delete;

  // Functors and lambdas. Binds by reference; a temporary passed here only
  // lives until the end of the full expression the FunctionRef was created in.
  template <typename TFunctor>
    requires(!ISSame<typename remove_reference<TFunctor>::type, FunctionRef> &&
             !ISSame<typename remove_reference<TFunctor>::type, const FunctionRef> &&
             !is_pointer<typename remove_reference<TFunctor>::type>::value)
  FunctionRef(TFunctor&& functor) noexcept
      : callable_((void*)AddressOf(functor)),
        invoke_([](void* callable, TArgs... args) -> TReturn {
          return (*static_cast<typename remove_reference<TFunctor>::type*>(
              callable))(forward<TArgs>(args)...);
        }) {}

  // Free functions.
  FunctionRef(TReturn (*function)(TArgs...)) noexcept
      : callable_(reinterpret_cast<void*>(function)),
        invoke_([](void* callable, TArgs... args) -> TReturn {
          return reinterpret_cast<TReturn (*)(TArgs...)>(callable)(
              forward<TArgs>(args)...);
        }) {}

  TReturn operator()(TArgs... args) const {
    return invoke_(callable_, forward<TArgs>(args)...);
  }

 private:
  void* callable_;
  TReturn (*invoke_)(void*, TArgs...);
};
}  // namespace base
