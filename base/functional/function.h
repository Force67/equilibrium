// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>
#include <base/check.h>
#include <new>
#include <cstring>

namespace base {

template <typename>
struct Function;  // intentionally not defined

// Type-erased callable wrapper, similar to std::function but without STL.
// Uses small buffer optimization (SBO) for callables up to 48 bytes.
template <typename R, typename... Args>
struct Function<R(Args...)> {
 private:
  static constexpr mem_size kSBOCapacity = 48;

  using InvokeFn = R (*)(void*, Args...);
  using DestroyFn = void (*)(void*);
  using MoveFn = void (*)(void* dst, void* src);

  InvokeFn invoke_{nullptr};
  DestroyFn destroy_{nullptr};
  MoveFn move_fn_{nullptr};

  alignas(8) unsigned char storage_[kSBOCapacity];
  void* target_{nullptr};  // points to storage_ or heap

  void Cleanup() {
    if (target_ && destroy_) {
      destroy_(target_);
      if (target_ != &storage_[0]) {
        base::DefaultAllocator::Free(target_, 0);
      }
    }
    target_ = nullptr;
    invoke_ = nullptr;
    destroy_ = nullptr;
    move_fn_ = nullptr;
  }

  template <typename T>
  static R InvokeImpl(void* target, Args... args) {
    return (*static_cast<T*>(target))(base::forward<Args>(args)...);
  }

  template <typename T>
  static void DestroyImpl(void* target) {
    static_cast<T*>(target)->~T();
  }

  template <typename T>
  static void MoveImpl(void* dst, void* src) {
    ::new (dst) T(base::move(*static_cast<T*>(src)));
    static_cast<T*>(src)->~T();
  }

  template <typename C, typename MF>
  struct MemberFuncHolder {
    C* object;
    MF member_func;
    MemberFuncHolder(C* obj, MF mf) : object(obj), member_func(mf) {}
  };

  template <typename C, typename MF>
  static R MemberInvokeImpl(void* target, Args... args) {
    auto* holder = static_cast<MemberFuncHolder<C, MF>*>(target);
    return (holder->object->*holder->member_func)(base::forward<Args>(args)...);
  }

 public:
  Function() {
    memset(storage_, 0, sizeof(storage_));
  }

  // Constructor for functors and lambdas
  template <typename T>
  Function(T&& target) {
    using Decayed = typename base::remove_reference<T>::type;
    memset(storage_, 0, sizeof(storage_));

    invoke_ = &InvokeImpl<Decayed>;
    destroy_ = &DestroyImpl<Decayed>;
    move_fn_ = &MoveImpl<Decayed>;

    if constexpr (sizeof(Decayed) <= kSBOCapacity) {
      target_ = &storage_[0];
      ::new (target_) Decayed(base::forward<T>(target));
    } else {
      target_ = base::DefaultAllocator::Allocate(sizeof(Decayed));
      ::new (target_) Decayed(base::forward<T>(target));
    }
  }

  // Constructor for member functions
  template <typename C, typename MF>
  Function(C* object, MF member_func) {
    using Holder = MemberFuncHolder<C, MF>;
    memset(storage_, 0, sizeof(storage_));

    invoke_ = reinterpret_cast<InvokeFn>(&MemberInvokeImpl<C, MF>);
    destroy_ = &DestroyImpl<Holder>;
    move_fn_ = &MoveImpl<Holder>;

    if constexpr (sizeof(Holder) <= kSBOCapacity) {
      target_ = &storage_[0];
      ::new (target_) Holder(object, member_func);
    } else {
      target_ = base::DefaultAllocator::Allocate(sizeof(Holder));
      ::new (target_) Holder(object, member_func);
    }
  }

  // Move constructor
  Function(Function&& other) noexcept {
    memset(storage_, 0, sizeof(storage_));
    invoke_ = other.invoke_;
    destroy_ = other.destroy_;
    move_fn_ = other.move_fn_;

    if (other.target_) {
      if (other.target_ == &other.storage_[0]) {
        // SBO: move-construct into our storage
        target_ = &storage_[0];
        if (move_fn_)
          move_fn_(target_, other.target_);
      } else {
        // Heap: just steal the pointer
        target_ = other.target_;
      }
    } else {
      target_ = nullptr;
    }

    other.target_ = nullptr;
    other.invoke_ = nullptr;
    other.destroy_ = nullptr;
    other.move_fn_ = nullptr;
  }

  Function& operator=(Function&& other) noexcept {
    if (this != &other) {
      Cleanup();

      invoke_ = other.invoke_;
      destroy_ = other.destroy_;
      move_fn_ = other.move_fn_;

      if (other.target_) {
        if (other.target_ == &other.storage_[0]) {
          target_ = &storage_[0];
          if (move_fn_)
            move_fn_(target_, other.target_);
        } else {
          target_ = other.target_;
        }
      } else {
        target_ = nullptr;
      }

      other.target_ = nullptr;
      other.invoke_ = nullptr;
      other.destroy_ = nullptr;
      other.move_fn_ = nullptr;
    }
    return *this;
  }

  ~Function() { Cleanup(); }

  R operator()(Args... args) const {
    BASE_DCHECK(invoke_ && target_, "Function: calling empty function");
    return invoke_(target_, base::forward<Args>(args)...);
  }

  explicit operator bool() const noexcept { return target_ != nullptr; }
  bool empty() const noexcept { return target_ == nullptr; }

  // Delete copy semantics
  Function(const Function&) = delete;
  Function& operator=(const Function&) = delete;
};
}  // namespace base
