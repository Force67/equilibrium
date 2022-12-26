// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>

namespace base {

template <typename T>
class Function;

template <class R, class... Args>
class Function<R(Args...)> {
  // Internal function type used to pass a closure object.
  // using InternalFunctionType = R(void*, Args...);
  typedef R InternalFunctionType(void*, Args&&... args);
  // using InternalFunctionType = R(*)(void*, Args&&... args);
  //  Internal function type used to delete a closure object.
  typedef void InternalDeleterFunctionType(void*);

 public:
  explicit Function(std::nullptr_t) {
    functor_ = [](void*, Args&&...) -> R {
      if constexpr (std::is_same<R, void>::value == false) {
        return R{};
      }
    };

    deleter_ = [](void*) {};
  }

  // Constructs a function from any callable type (free function, captureless lambda,
  // capturing lambda).
  template <class T>
  explicit Function(T&& closure) {
    // convert the given (potentially capturing) closure to a captureless lambda by
    // passing the closure as the context object
    functor_ = [](void* closure, Args&&... args) -> R {
      T* realClosure = static_cast<T*>(closure);
      return (*realClosure)(base::forward<Args>(args)...);
    };

    // make sure to allocate and move the closure object onto the heap, as it will
    // very likely be out-of-scope when this function is called
    closure_context_ = new T(base::forward<T>(closure));

    // create a deleter function that is capable of deleting the heap-allocated
    // closure object
    deleter_ = [](void* closure) {
      T* realClosure = static_cast<T*>(closure);
      delete realClosure;
    };
  }

  // Deletes allocated resources.
  ~Function(void) { (*deleter_)(closure_context_); }

  // Invokes the underlying callable function object.
  inline R operator()(Args... args) const {
    return (*functor_)(closure_context_, base::forward<Args>(args)...);
  }

 private:
  InternalFunctionType* functor_ = nullptr;
  InternalDeleterFunctionType* deleter_ = nullptr;
  void* closure_context_ = nullptr;

  BASE_NOCOPYMOVE(Function);
  // LC_DISABLE_COPY_ASSIGNMENT(Function);
  // LC_DISABLE_MOVE_ASSIGNMENT(Function);
};
}  // namespace base