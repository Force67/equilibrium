// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>

namespace base {

template <typename Ta, typename Tb>
struct Pair {
  Ta a;
  Tb b;

  // STL-compat aliases
  Ta& first() { return a; }
  const Ta& first() const { return a; }
  Tb& second() { return b; }
  const Tb& second() const { return b; }
};

template <typename Ta, typename Tb>
Pair<Ta, Tb> MakePair(Ta a, Tb b) {
  return Pair<Ta, Tb>{base::move(a), base::move(b)};
}
}  // namespace base
