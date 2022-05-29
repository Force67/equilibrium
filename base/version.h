#pragma once
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>

namespace base {

// from
// https://stackoverflow.com/questions/69468895/reverse-order-of-varidic-template-arguments-while-constructing-a-map-key
template <typename TRet, typename... Args>
constexpr TRet MakeVersionKey(TRet base, Args... args) {
  TRet res{};
  // basically does major * 1000000 + minor * 10000 + patch * 100 + pp;
  ((res *= base, res += args % base), ...);
  return res;
}
static_assert(MakeVersionKey<mem_size>(100, 1, 2, 3) == 10203);

template <typename... Args>
constexpr void ExtractFromVersionKey(auto base, auto key, Args&... args) {
  auto step = 1;
  // this dummy exists because we dont have a nice way to do this in reverse yet:
  // https://quuxplusone.github.io/blog/2020/05/07/assignment-operator-fold-expression/
  int dummy = 0;
  ((args = key / step % base, step *= base, dummy) = ... = 0);
}
}  // namespace base