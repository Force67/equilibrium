// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Exposes RETK plugin bindings to IDC and IDA-python.
#pragma once

#include <functional>
#include <expr.hpp>

// Add IDC class

// TODO: custom free object
#if 0
class ScriptClass {
 public:
  virtual ~ScriptClass() = default;
};

class ScriptClassBinding final {
 public:
  ScriptClassBinding(const char* const name) : name_(name) {}

  void BindThatShit() {
    auto* idc_cvt_opaque = add_idc_class(name_);
    if (idc_cvt_opaque == nullptr) {
      __debugbreak();
    }

    add_idc_func(kDTorDesc);
    set_idc_dtor(idc_cvt_opaque, kDTorDesc.name);

    set_idc_method();
  }

 private:
  // Destruct the underlying cxx object
  static error_t idaapi DTorImpl(idc_value_t* argv, idc_value_t* /*res*/) {
    idc_value_t idc_val;
    get_idcv_attr(&idc_val, &argv[0], kIdcvName);

    ScriptClass* user_object = static_cast<ScriptClass*>(idc_val.pvoid);
    delete user_object;

    return eOk;
  }

 private:
  static constexpr char kIdcvName[] = "__idc_retk_cxx_obj_value__";
  static constexpr char kDTorArgs[] = {VT_OBJ, 0};
  static constexpr ext_idcfunc_t kDTorDesc = {};
  const char* const name_;
};
#endif

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

template <int unique_id, typename TRet, typename... Ts>
class ScriptBinding final : public BindingBase {
 public:
  using TFunc = TRet(*)(Ts...);
  static constexpr size_t N = sizeof...(Ts);
  static inline int ID = unique_id;

  ScriptBinding(const char* const name, TFunc function)
      : BindingBase(name, args_, FunctorImpl),
        // create the type index list by converting each argument
        //  into its numeric representation, note that we need to store
        //  a null terminator for book-keeping at the end.
        args_{(ToValueTypeIndex<Ts>(), ...), 0} {
    functor_ = std::move(function);
  }

  inline void Invoke(Ts... args) { functor_(args...); }

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
    return functor_(ToConreteValueType<Ts>(t[I])...);
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
  // marking this type as non static to avoid duplication
  static inline TFunc functor_;
};