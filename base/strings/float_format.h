// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Exact double-to-text conversion, so base does not need snprintf to format
// a float.
//
// Every finite double is exactly representable in decimal, because 2^-n is,
// so there is no approximation anywhere here: the digits are computed exactly
// and then rounded once, at the requested position, half-to-even. That is
// what printf does, and float_format_test.cc checks the two against each
// other over millions of values rather than taking it on faith.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

// Writes |value| in printf's style into |out|, never writing more than
// |capacity| bytes and never null-terminating. Returns the length the
// conversion needs, which may exceed |capacity|. The caller checks, as it
// would with snprintf.
//
//   type       'f', 'e' or 'g', or uppercase for INF/NAN and a capital E
//   precision  digits after the point for 'f'/'e', significant digits for
//              'g'; negative selects printf's default of 6
//   sign       what to emit ahead of a non-negative value: '\0', '+' or ' '
//   alt        printf's '#': keep the point, and the trailing zeros in 'g'
BASE_EXPORT mem_size FormatFloatTo(char* out, mem_size capacity, f64 value,
                                   char type, int precision, char sign,
                                   bool alt) noexcept;

// The longest output FormatFloatTo can produce for a default precision, and
// a safe stack buffer size for the common cases. A caller asking for a large
// explicit precision needs more and should use the returned length.
inline constexpr mem_size kFloatFormatBufferSize = 512;

}  // namespace base
