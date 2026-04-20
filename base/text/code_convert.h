// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Unicode conversions between UTF-8, UTF-16, wchar_t, and 7-bit ASCII.
// Pointer-form APIs write into a caller-supplied output string and return
// false when the input contained malformed sequences (the malformed parts
// are replaced with U+FFFD, so the output is always well-formed). Value-form
// APIs always succeed and substitute U+FFFD on the way.
#pragma once

#include <base/arch.h>
#include <base/export.h>
#include <base/strings/string_ref.h>
#include <base/strings/xstring.h>

namespace base {

// UTF-8 <-> UTF-16.
BASE_EXPORT bool UTF8ToUTF16(const char* src, mem_size src_len,
                             base::StringU16* output);
[[nodiscard]] BASE_EXPORT base::StringU16 UTF8ToUTF16(base::StringRef utf8);

BASE_EXPORT bool UTF16ToUTF8(const char16_t* src, mem_size src_len,
                             base::StringU8* output);
[[nodiscard]] BASE_EXPORT base::StringU8 UTF16ToUTF8(base::StringRefU16 utf16);

// UTF-8 <-> wide.
BASE_EXPORT bool UTF8ToWide(const char* src, mem_size src_len,
                            base::StringW* output);
BASE_EXPORT bool UTF8ToWide(const char8_t* src, mem_size src_len,
                            base::StringW* output);
[[nodiscard]] BASE_EXPORT base::StringW UTF8ToWide(base::StringRefU8 utf8);

BASE_EXPORT bool WideToUTF8(const wchar_t* src, mem_size src_len,
                            base::StringU8* output);
[[nodiscard]] BASE_EXPORT base::StringU8 WideToUTF8(base::StringRefW wide);

// UTF-16 <-> wide. On Windows (UTF-16 wchar_t) this is a memcpy; on POSIX
// (UTF-32 wchar_t) it transcodes through codepoints.
BASE_EXPORT bool WideToUTF16(const wchar_t* src, mem_size src_len,
                             base::StringU16* output);
[[nodiscard]] BASE_EXPORT base::StringU16 WideToUTF16(base::StringRefW wide);

BASE_EXPORT bool UTF16ToWide(const char16_t* src, mem_size src_len,
                             base::StringW* output);
[[nodiscard]] BASE_EXPORT base::StringW UTF16ToWide(base::StringRefU16 utf16);

// Zero-extends each byte of an ASCII string into a wide string. Asserts in
// debug builds that the input is pure 7-bit ASCII.
[[nodiscard]] BASE_EXPORT base::StringW ASCIIToWide(base::StringRef ascii);

// Truncates each wide char to its low 8 bits. The caller is responsible for
// ensuring the input is pure 7-bit ASCII; this is checked with a BUGCHECK.
[[nodiscard]] BASE_EXPORT base::String WideToASCII(base::StringRefW wide);

}  // namespace base
