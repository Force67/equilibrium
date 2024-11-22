#pragma once

#include <base/memory/move.h>
#include <memory>

namespace base {
template <typename>
struct Function;  // intentionally not defined

template <typename T>
struct Decay {
  using type = T;  // This should be more complex in a full implementation
};

/*
Usage:
  MyClass myObject;
  Function<int(int)> myFunc(&myObject, &MyClass::memberFunction);
  int result = myFunc(2);

  Function<int(int)> myFreeFunc(&freefn);
  result = myFreeFunc(12);

  Function<int(int)> ref = base::move(myFunc);
  result = ref(2);
*/
template <typename R, typename... Args>
struct Function<R(Args...)> {
  using Dispatcher = R (*)(void*, Args...);

  Dispatcher m_Dispatcher;
  std::unique_ptr<void, void (*)(void*)> m_Target;

  // Custom deleter for unique_ptr
  template <typename T>
  static void Deleter(void* ptr) {
    delete static_cast<T*>(ptr);
  }

  template <typename S>
  static R Dispatch(void* target, Args... args) {
    return (*static_cast<S*>(target))(base::forward<Args>(args)...);
  }

  // Custom structure to hold class instance and member function pointer
  template <typename C, typename MF>
  struct MemberFuncHolder {
    C* object;
    MF memberFuncPtr;

    MemberFuncHolder(C* obj, MF mfPtr) : object(obj), memberFuncPtr(mfPtr) {}
  };

  // Dispatch for member functions
  template <typename C, typename MF>
  static R MemberDispatch(void* target, Args... args) {
    auto holder = static_cast<MemberFuncHolder<C, MF>*>(target);
    return (holder->object->*holder->memberFuncPtr)(base::forward<Args>(args)...);
  }

  // Constructor for functors and free functions
  template <typename T>
  Function(T&& target)
      : m_Dispatcher(&Dispatch<typename Decay<T>::type>),
        m_Target(new typename Decay<T>::type(base::forward<T>(target)),
                 &Deleter<typename Decay<T>::type>) {}

  // Constructor for member functions
  template <typename C, typename MF>
  Function(C* object, MF memberFuncPtr)
      : m_Dispatcher(reinterpret_cast<Dispatcher>(&MemberDispatch<C, MF>)),
        m_Target(new MemberFuncHolder<C, MF>(object, memberFuncPtr),
                 &Deleter<MemberFuncHolder<C, MF>>) {}

  // Move constructor and assignment
  Function(Function&& other) noexcept
      : m_Dispatcher(other.m_Dispatcher), m_Target(base::move(other.m_Target)) {}

  Function& operator=(Function&& other) noexcept {
    if (this != &other) {
      m_Dispatcher = other.m_Dispatcher;
      m_Target = base::move(other.m_Target);
    }
    return *this;
  }

  // Call operator
  R operator()(Args... args) const {
    return m_Dispatcher(m_Target.get(), base::forward<Args>(args)...);
  }

  // Delete copy semantics
  Function(const Function&) = delete;
  Function& operator=(const Function&) = delete;
};
}  // namespace base
