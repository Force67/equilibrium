// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>
#include <base/check.h>
#include <base/containers/container_traits.h>
#include <new>
#include <cstring>
#include <cstddef>  // max_align_t

namespace base {

template <typename>
struct Function;  // intentionally not defined

// Type-erased callable wrapper. Full std::function replacement.
// - Copyable (like std::function)
// - Small buffer optimization (SBO) for callables up to 48 bytes
// - No STL dependencies
template <typename R, typename... Args>
struct Function<R(Args...)> {
 private:
  static constexpr mem_size kSBOCapacity = 48;

  using InvokeFn = R (*)(const void*, Args...);
  using DestroyFn = void (*)(void*);
  using CopyFn = void (*)(void* dst, const void* src);
  using MoveFn = void (*)(void* dst, void* src);
  using SizeFn = mem_size (*)();

  InvokeFn invoke_{nullptr};
  DestroyFn destroy_{nullptr};
  CopyFn copy_fn_{nullptr};
  MoveFn move_fn_{nullptr};
  SizeFn size_fn_{nullptr};

  alignas(alignof(max_align_t)) unsigned char storage_[kSBOCapacity];
  void* heap_ptr_{nullptr};  // non-null when target is on the heap
  bool has_target_{false};

  bool IsSBO() const { return has_target_ && heap_ptr_ == nullptr; }
  void* Target() { return IsSBO() ? static_cast<void*>(&storage_[0]) : heap_ptr_; }
  const void* Target() const { return IsSBO() ? static_cast<const void*>(&storage_[0]) : heap_ptr_; }

  void Cleanup() {
    if (has_target_ && destroy_) {
      destroy_(Target());
      if (heap_ptr_) {
        base::DefaultAllocator::Free(heap_ptr_, 0);
        heap_ptr_ = nullptr;
      }
    }
    has_target_ = false;
    invoke_ = nullptr;
    destroy_ = nullptr;
    copy_fn_ = nullptr;
    move_fn_ = nullptr;
    size_fn_ = nullptr;
  }

  template <typename T>
  static R InvokeImpl(const void* target, Args... args) {
    return (*const_cast<T*>(static_cast<const T*>(target)))(
        base::forward<Args>(args)...);
  }

  template <typename T>
  static void DestroyImpl(void* target) {
    static_cast<T*>(target)->~T();
  }

  template <typename T>
  static void CopyImpl(void* dst, const void* src) {
    ::new (dst) T(*static_cast<const T*>(src));
  }

  template <typename T>
  static void MoveImpl(void* dst, void* src) {
    ::new (dst) T(base::move(*static_cast<T*>(src)));
    static_cast<T*>(src)->~T();
  }

  template <typename T>
  static mem_size SizeImpl() { return sizeof(T); }

  template <typename C, typename MF>
  struct MemberFuncHolder {
    C* object;
    MF member_func;
    MemberFuncHolder(C* obj, MF mf) : object(obj), member_func(mf) {}
  };

  template <typename C, typename MF>
  static R MemberInvokeImpl(const void* target, Args... args) {
    auto* holder = const_cast<MemberFuncHolder<C, MF>*>(
        static_cast<const MemberFuncHolder<C, MF>*>(target));
    return (holder->object->*holder->member_func)(base::forward<Args>(args)...);
  }

  // Proper decay: function types become function pointers
  template <typename T>
  struct FnDecay { using type = typename base::remove_reference<T>::type; };
  template <typename Ret, typename... A>
  struct FnDecay<Ret(A...)> { using type = Ret(*)(A...); };
  template <typename Ret, typename... A>
  struct FnDecay<Ret(&)(A...)> { using type = Ret(*)(A...); };
  template <typename Ret, typename... A>
  struct FnDecay<Ret(&&)(A...)> { using type = Ret(*)(A...); };

  template <typename D>
  void InitVTable() {
    invoke_ = &InvokeImpl<D>;
    destroy_ = &DestroyImpl<D>;
    copy_fn_ = &CopyImpl<D>;
    move_fn_ = &MoveImpl<D>;
    size_fn_ = &SizeImpl<D>;
  }

  template <typename D, typename U>
  void Emplace(U&& obj) {
    InitVTable<D>();
    if constexpr (sizeof(D) <= kSBOCapacity &&
                  alignof(D) <= alignof(max_align_t)) {
      heap_ptr_ = nullptr;
      ::new (&storage_[0]) D(base::forward<U>(obj));
    } else {
      heap_ptr_ = base::DefaultAllocator::Allocate(sizeof(D));
      ::new (heap_ptr_) D(base::forward<U>(obj));
    }
    has_target_ = true;
  }

  void CopyFrom(const Function& other) {
    invoke_ = other.invoke_;
    destroy_ = other.destroy_;
    copy_fn_ = other.copy_fn_;
    move_fn_ = other.move_fn_;
    size_fn_ = other.size_fn_;
    has_target_ = other.has_target_;

    if (other.has_target_ && other.copy_fn_) {
      if (other.IsSBO()) {
        // Copy into our SBO storage
        heap_ptr_ = nullptr;
        other.copy_fn_(&storage_[0], &other.storage_[0]);
      } else {
        // Heap: allocate and copy
        mem_size sz = other.size_fn_ ? other.size_fn_() : kSBOCapacity;
        heap_ptr_ = base::DefaultAllocator::Allocate(sz);
        other.copy_fn_(heap_ptr_, other.heap_ptr_);
      }
    } else {
      heap_ptr_ = nullptr;
    }
  }

  void MoveFrom(Function& other) {
    invoke_ = other.invoke_;
    destroy_ = other.destroy_;
    copy_fn_ = other.copy_fn_;
    move_fn_ = other.move_fn_;
    size_fn_ = other.size_fn_;
    has_target_ = other.has_target_;

    if (other.has_target_) {
      if (other.IsSBO()) {
        heap_ptr_ = nullptr;
        if (other.move_fn_)
          other.move_fn_(&storage_[0], &other.storage_[0]);
      } else {
        // Steal the heap pointer
        heap_ptr_ = other.heap_ptr_;
      }
    } else {
      heap_ptr_ = nullptr;
    }

    other.has_target_ = false;
    other.heap_ptr_ = nullptr;
    other.invoke_ = nullptr;
    other.destroy_ = nullptr;
    other.copy_fn_ = nullptr;
    other.move_fn_ = nullptr;
    other.size_fn_ = nullptr;
  }

 public:
  Function() { memset(storage_, 0, sizeof(storage_)); }

  // Nullptr construction / assignment / comparison
  Function(decltype(nullptr)) : Function() {}
  Function& operator=(decltype(nullptr)) { Cleanup(); return *this; }
  bool operator==(decltype(nullptr)) const noexcept { return !has_target_; }
  bool operator!=(decltype(nullptr)) const noexcept { return has_target_; }

  // SFINAE helper: true for everything except Function itself and nullptr_t
  template <typename T>
  static constexpr bool IsCallable =
      !base::ISSame<typename base::remove_reference<T>::type, Function> &&
      !base::ISSame<typename base::remove_reference<T>::type, decltype(nullptr)>;

  // Constructor from callable (lambda, functor, function pointer)
  template <typename T>
    requires(IsCallable<T>)
  Function(T&& target) {
    memset(storage_, 0, sizeof(storage_));
    using Decayed = typename FnDecay<T>::type;
    Emplace<Decayed>(base::forward<T>(target));
  }

  // Assignment from callable
  template <typename T>
    requires(IsCallable<T>)
  Function& operator=(T&& target) {
    Cleanup();
    using Decayed = typename FnDecay<T>::type;
    Emplace<Decayed>(base::forward<T>(target));
    return *this;
  }

  // Constructor for member functions
  template <typename C, typename MF>
  Function(C* object, MF member_func) {
    using Holder = MemberFuncHolder<C, MF>;
    memset(storage_, 0, sizeof(storage_));

    // Set vtable manually — invoke uses MemberInvokeImpl, rest uses Holder
    invoke_ = reinterpret_cast<InvokeFn>(&MemberInvokeImpl<C, MF>);
    destroy_ = &DestroyImpl<Holder>;
    copy_fn_ = &CopyImpl<Holder>;
    move_fn_ = &MoveImpl<Holder>;
    size_fn_ = &SizeImpl<Holder>;

    if constexpr (sizeof(Holder) <= kSBOCapacity) {
      heap_ptr_ = nullptr;
      ::new (&storage_[0]) Holder(object, member_func);
    } else {
      heap_ptr_ = base::DefaultAllocator::Allocate(sizeof(Holder));
      ::new (heap_ptr_) Holder(object, member_func);
    }
    has_target_ = true;
  }

  // Copy constructor
  Function(const Function& other) {
    memset(storage_, 0, sizeof(storage_));
    CopyFrom(other);
  }

  // Copy assignment (copy-and-swap)
  Function& operator=(const Function& other) {
    if (this != &other) {
      Cleanup();
      memset(storage_, 0, sizeof(storage_));
      CopyFrom(other);
    }
    return *this;
  }

  // Move constructor
  Function(Function&& other) noexcept {
    memset(storage_, 0, sizeof(storage_));
    MoveFrom(other);
  }

  // Move assignment
  Function& operator=(Function&& other) noexcept {
    if (this != &other) {
      Cleanup();
      memset(storage_, 0, sizeof(storage_));
      MoveFrom(other);
    }
    return *this;
  }

  ~Function() { Cleanup(); }

  void Swap(Function& other) {
    Function tmp(base::move(other));
    other = base::move(*this);
    *this = base::move(tmp);
  }

  void Reset() { Cleanup(); }

  R operator()(Args... args) const {
    BASE_DCHECK(invoke_ && has_target_, "Function: calling empty function");
    return invoke_(Target(), base::forward<Args>(args)...);
  }

  explicit operator bool() const noexcept { return has_target_; }
  bool empty() const noexcept { return !has_target_; }
};
}  // namespace base
