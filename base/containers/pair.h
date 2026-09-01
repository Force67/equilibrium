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

// Lexicographic ordering, so a Pair can key an ordered container.
template <typename Ta, typename Tb>
constexpr bool operator==(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  return a.first == b.first && a.second == b.second;
}

template <typename Ta, typename Tb>
constexpr bool operator!=(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  return !(a == b);
}

template <typename Ta, typename Tb>
constexpr bool operator<(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  if (a.first < b.first) return true;
  if (b.first < a.first) return false;
  return a.second < b.second;
}

template <typename Ta, typename Tb>
constexpr bool operator>(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  return b < a;
}

template <typename Ta, typename Tb>
constexpr bool operator<=(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  return !(b < a);
}

template <typename Ta, typename Tb>
constexpr bool operator>=(const Pair<Ta, Tb>& a, const Pair<Ta, Tb>& b) {
  return !(a < b);
}

template <typename Ta, typename Tb>
constexpr Pair<base::remove_reference_t<Ta>, base::remove_reference_t<Tb>>
MakePair(Ta&& a, Tb&& b) {
  return {base::forward<Ta>(a), base::forward<Tb>(b)};
}
}  // namespace base
