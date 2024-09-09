// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

template <typename Ta, typename Tb>
struct Pair {
  Ta a;
  Tb b;
};

template <typename Ta, typename Tb>
Pair<Ta, Tb> MakePair(Ta a, Tb b) {
  return Pair<Ta, Tb>{a, b};
}
}  // namespace base