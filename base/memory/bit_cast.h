// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

// The object representation of `from`, read as a `To` of the same size.
template <typename To, typename From>
constexpr To BitCast(const From& from) noexcept {
  static_assert(sizeof(To) == sizeof(From), "BitCast needs equal sizes");
  return __builtin_bit_cast(To, from);
}

}  // namespace base
