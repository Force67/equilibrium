// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {
template <class T>
inline constexpr bool IsTrivial =
    __is_trivially_constructible(T) && __is_trivially_copyable(T);

template <typename>
inline constexpr bool IsArray = false;  // determine whether type argument is an array

template <typename T, mem_size TSize>
inline constexpr bool IsArray<T[TSize]> = true;

template <typename T>
inline constexpr bool IsArray<T[]> = true;

template <typename, typename>
inline constexpr bool ISSame = false;  // determine whether arguments are the same type
template <typename T>
inline constexpr bool ISSame<T, T> = true;

template <class _Ty>
struct remove_extent {  // remove array extent
  using type = _Ty;
};
template <class _Ty, mem_size _Ix>
struct remove_extent<_Ty[_Ix]> {
  using type = _Ty;
};
template <class _Ty>
struct remove_extent<_Ty[]> {
  using type = _Ty;
};
template <class _Ty>
using remove_extent_t = typename remove_extent<_Ty>::type;

template <bool _Test, class _Ty1, class _Ty2>
struct conditional {  // Choose _Ty1 if _Test is true, and _Ty2 otherwise
  using type = _Ty1;
};

template <class _Ty1, class _Ty2>
struct conditional<false, _Ty1, _Ty2> {
  using type = _Ty2;
};

template <bool _Test, class _Ty1, class _Ty2>
using conditional_t = typename conditional<_Test, _Ty1, _Ty2>::type;

// std::destroy_at(&str);

template <typename T>
inline void destruct(T* p) {
  // https://msdn.microsoft.com/query/dev14.query?appId=Dev14IDEF1&l=EN-US&k=k(C4100)&rd=true
  // "C4100 can also be issued when code calls a destructor on a otherwise
  // unreferenced parameter
  //  of primitive type. This is a limitation of the Visual C++ compiler."
  // EA_UNUSED(p);
  p->~T();
}

template <typename TType>
inline void DestructRange(TType first, TType last) {
  // if constexpr (!IsTrivial<T>) {
  for (; first != last; ++first) {
    destruct(first);
    // first->~Type();
    //(*first).~TT();
  }
  //  }
}

}  // namespace base