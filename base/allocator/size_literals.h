// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base::size_literals {
// Please avoid using these for any memory specific calculations use the *bi methods
// instead
constexpr size_t operator""_kb(size_t x) {
  return 1000ULL * x;
}
constexpr size_t operator""_mb(size_t x) {
  return 1000_kb * x;
}
constexpr size_t operator""_gb(size_t x) {
  return 1000_mb * x;
}
constexpr size_t operator""_tb(size_t x) {
  return 1000_gb * x;
}
constexpr size_t operator""_pb(size_t x) {
  return 1000_tb * x;
}

// *bi bytes
constexpr size_t operator""_kib(size_t x) {
  return 1024ULL * x;
}
constexpr size_t operator""_mib(size_t x) {
  return 1024_kib * x;
}
constexpr size_t operator""_gib(size_t x) {
  return 1024_mib * x;
}
constexpr size_t operator""_tib(size_t x) {
  return 1024_gib * x;
}
constexpr size_t operator""_pib(size_t x) {
  return 1024_tib * x;
}
}  // namespace base::size_literals