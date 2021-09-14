// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <expr.hpp>

namespace iretk {

inline static void Test1(const char* a, int64_t b) {}

//   const char args_[3] = {VT_INT64, VT_STR, 0};

class BindingBase {
 public:
  inline BindingBase(const char* const name,
                     const char* const args,
                     idc_func_t* funcptr) {
    desc_ = {name, funcptr, args, nullptr, 0, 0};
    next_ = this;
  }

  static void BindAll();
  static void UnBindAll();

 private:
  BindingBase* next_;
  ext_idcfunc_t desc_;
};

template <size_t N, typename TFunctor, typename... Ts>
class Binding final : public BindingBase {
 public:
  inline explicit Binding(TFunctor &funcRef, const char* const name)
      : BindingBase(name, args_, Functor),
        args_{(ToValueTypeIndex<Ts>(), ...), 0} {
    functor_ = &funcRef;
  }

 private:
  static error_t idaapi Functor(idc_value_t* argv, idc_value_t* result) {
    InvokeFunctor(argv, std::make_index_sequence<N>{});
    return eOk;
  }

  template <size_t... I>
  static inline void InvokeFunctor(idc_value_t* t, std::index_sequence<I...>) {
    functor_(ToConreteValueType<Ts>(t[I])...);
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
  static inline TFunctor *functor_;
};

using XT = decltype(Test1);

static Binding<2, XT, const char*, int64_t> kTest(Test1, "LMAO");
}  // namespace iretk