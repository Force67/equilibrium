// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <expr.hpp>

// Base class so the Binding can self (de)-register
// during it's lifetime
class BindingBase {
 public:
  BindingBase(const char* const name,
                     const char* const args,
                     idc_func_t* funcptr);

  static void BindAll();
  static void UnBindAll();

 private:
  BindingBase* next_;
  ext_idcfunc_t desc_;
};

template <typename TRet, typename... Ts>
class Binding final : public BindingBase {
 public:
  using TFunctor = TRet(Ts...);
  static constexpr size_t N = sizeof...(Ts);

  template <typename TF>
  Binding(const char* const name, TF function)
      : BindingBase(name, args_, FunctorImpl),
    // create the type index list by converting each argument
    //  into its numeric representation, note that we need to store
    //  a null terminator for book-keeping at the end.
        args_{(ToValueTypeIndex<Ts>(), ...), 0} {
    // decay type so we can support lambda syntax
    functor_ = &function;
  }

  void ExecuteFunctor(Ts... args) { 
      reinterpret_cast<TFunctor*>(functor_)(args...);
  }

 private:
  static error_t idaapi FunctorImpl(idc_value_t* argv, idc_value_t* result) {
    if constexpr (std::is_same_v<TRet, void>) {
      InvokeFunctor(argv, std::make_index_sequence<N>{});
      return eOk;
    }

    // this works due the constructor if idc_value_t getting invoked here
    *result = InvokeFunctor(argv, std::make_index_sequence<N>{});
    return eOk;
  }

  template <size_t... I>
  static inline TRet InvokeFunctor(idc_value_t* t, std::index_sequence<I...>) {
    // reconstruct type pointer and flatten argv into function args 
    // e.g array<a,b,c> -> f(a,b,c)
    return reinterpret_cast<TFunctor*>(functor_)(ToConreteValueType<Ts>(t[I])...);
  }

  template <typename T>
  static constexpr char ToValueTypeIndex() {
    if constexpr (std::is_same_v<T, int64_t>)
      return VT_INT64;
    if constexpr (std::is_same_v<T, const char*>)
      return VT_STR;
    if constexpr (std::is_same_v<T, sval_t>)
      return VT_LONG;
    return VT_PVOID;
  }

  template <typename T>
  static constexpr T ToConreteValueType(idc_value_t& value) {
    if constexpr (std::is_same_v<T, int64_t>)
      return value.i64;
    if constexpr (std::is_same_v<T, const char*>)
      return value.c_str();
    if constexpr (std::is_same_v<T, sval_t>)
      return value.num;
    return reinterpret_cast<T>(value.pvoid);
  }

 private:
  // +1 since we need to store a null terminator at the end
  const char args_[N + 1];
  static inline void* functor_;
};