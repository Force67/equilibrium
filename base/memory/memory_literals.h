// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base::memory_literals {
// Please avoid using these for any memory specific calculations use the *ib
// methods instead
constexpr mem_size operator""_kb(mem_size x) {
  return 1000ULL * x;
}
constexpr mem_size operator""_mb(mem_size x) {
  return 1000_kb * x;
}
constexpr mem_size operator""_gb(mem_size x) {
  return 1000_mb * x;
}
constexpr mem_size operator""_tb(mem_size x) {
  return 1000_gb * x;
}
constexpr mem_size operator""_pb(mem_size x) {
  return 1000_tb * x;
}

// *bi bytes
constexpr mem_size operator""_kib(mem_size x) {
  return 1024ULL * x;
}
constexpr mem_size operator""_mib(mem_size x) {
  return 1024_kib * x;
}
constexpr mem_size operator""_gib(mem_size x) {
  return 1024_mib * x;
}
constexpr mem_size operator""_tib(mem_size x) {
  return 1024_gib * x;
}
constexpr mem_size operator""_pib(mem_size x) {
  return 1024_tib * x;
}
}  // namespace base::memory_literals