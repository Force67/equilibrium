// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Text-to-number conversion, so base does not need strtoll or strtod.
//
// These follow the C functions closely enough to be drop-in for base's own
// callers: leading whitespace, an optional sign, the 0x and 0 prefixes for
// base detection. number_parse_test.cc checks them against those
// functions rather than against a reading of the standard. What they do not
// do is set errno or honour a locale: the decimal point is always '.'.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

// Parses a signed integer. |base_radix| is 2..36, or 0 to detect it from a
// 0x/0X prefix (16), a leading 0 (8), or otherwise 10.
//
// Returns false and leaves |out| untouched when no digits were found. On
// overflow it saturates to the type's limit and still returns true, as strtoll
// does. |end|, when given, receives the first character not consumed.
BASE_EXPORT bool ParseInteger(const char* text, i64& out,
                              int base_radix = 10,
                              const char** end = nullptr) noexcept;

// As ParseInteger, for unsigned values.
BASE_EXPORT bool ParseUnsigned(const char* text, u64& out,
                               int base_radix = 10,
                               const char** end = nullptr) noexcept;

// Parses a double, accepting fixed and scientific notation plus inf, infinity
// and nan in any case. The result is the correctly rounded nearest double --
// the digits are accumulated exactly and rounded once, half-to-even, not
// multiplied up in floating point where the error would compound.
//
// Returns false and leaves |out| untouched when the text is not a number.
// Overflow yields infinity and underflow zero, both returning true.
BASE_EXPORT bool ParseFloat(const char* text, f64& out,
                            const char** end = nullptr) noexcept;

}  // namespace base
