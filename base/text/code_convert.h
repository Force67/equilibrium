// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once
#include "base/strings/string_ref.h"
#include "base/export.h"
#include "base/compiler.h"

namespace base {

BASE_EXPORT bool WideToUTF8(const wchar_t* src,
                            size_t src_len,
                            base::String* output);

[[nodiscard]] BASE_EXPORT base::String WideToUTF8(base::StringRefW wide);

BASE_EXPORT bool UTF8ToWide(const char* src,
                            size_t src_len,
                            base::StringW* output);

[[nodiscard]] BASE_EXPORT base::StringW UTF8ToWide(base::StringRefU8 utf8);

BASE_EXPORT bool WideToUTF16(const wchar_t* src,
                             size_t src_len,
                             std::u16string* output);
BASE_EXPORT bool UTF16ToWide(const char16_t* src,
                             size_t src_len,
                             base::StringW* output);
BASE_EXPORT bool UTF8ToUTF16(const char* src,
                             size_t src_len,
                             std::u16string* output);
BASE_EXPORT bool UTF16ToUTF8(const char16_t* src,
                             size_t src_len,
                             base::String* output);

// This converts an ASCII string, typically a hardcoded constant, to a wide
// string.
[[nodiscard]] BASE_EXPORT base::StringW ASCIIToWide(base::StringRef ascii);

// Converts to 7-bit ASCII by truncating. The result must be known to be ASCII
// beforehand.
[[nodiscard]] BASE_EXPORT base::String WideToASCII(base::StringRefW wide);
}  // namespace base