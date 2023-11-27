#pragma once
#include <utility>  // For std::forward

namespace base {

template <typename Signature>
class FunctionRef;

template <typename TReturn, typename... Args>
class FunctionRef<TReturn(Args...)> final {
 public:
  using signature_type = TReturn(void*, Args...);
  using member_function_type = TReturn (*)(void*, Args...);

  // Existing constructor for functors and free functions
  template <typename Functor>
  FunctionRef(Functor&& functor)
      : object_(nullptr), functor_((void*)std::addressof(functor)) {
    erased_function_ = [](void* obj, void* ptr,
                          Args... args) -> decltype(auto) {
      return (*static_cast<Functor*>(ptr))(std::forward<Args>(args)...);
    };
  }

  // Special handling for member functions
  template <typename T, typename MemberFunction>
  FunctionRef(T* instance, MemberFunction memberFunction)
      : object_((void*)instance) {
    // Cast the member function to a type that can be stored in a void* pointer
    member_function_ = reinterpret_cast<member_function_type*>(&memberFunction);

    erased_function_ = [](void* obj, void* ptr, Args... args) -> TReturn {
      // Cast the object and function pointer back to their original types
      auto real_obj = static_cast<T*>(obj);
      auto real_func = reinterpret_cast<MemberFunction>(ptr);
      return (real_obj->*real_func)(std::forward<Args>(args)...);
    };
  }

  TReturn operator()(Args... args) noexcept {
#if 0
    if (object_) {
      // Member function call
      return erased_function_(object_, member_function_,
                              std::forward<Args>(args)...);
    } else {
      // Regular function call
      return erased_function_(nullptr, functor_, std::forward<Args>(args)...);
    }
#endif
    return {};
  }

 private:
  void* object_ = nullptr;  // For member functions, this is the object instance
  void* functor_;           // For regular functions and functors
  signature_type* erased_function_;
  member_function_type* member_function_;  // For member functions
};
}  // namespace base
