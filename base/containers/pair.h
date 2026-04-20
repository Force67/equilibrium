// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>

namespace base {

template <typename Ta, typename Tb>
struct Pair {
  Ta first;
  Tb second;
};

template <typename Ta, typename Tb>
constexpr Pair<base::remove_reference_t<Ta>, base::remove_reference_t<Tb>>
MakePair(Ta&& a, Tb&& b) {
  return {base::forward<Ta>(a), base::forward<Tb>(b)};
}
}  // namespace base
