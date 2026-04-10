// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// XString implementation - our optimized string class.
// Note that this file has to be called 'xstring.h' instead of 'string.h' for
// backwards compat reasons with the c header.
#pragma once

#include <base/export.h>
#include <base/strings/base_string.h>
#include <base/strings/fixed_string.h>
#include <base/strings/small_string.h>

namespace base {
template <typename T>
using XBasicString = BasicBaseString<T>;

// Core, heap-backed string types (24-byte SSO).
using String = XBasicString<char>;
using StringW = XBasicString<wchar_t>;
using StringU8 = XBasicString<char8_t>;
using StringU16 = XBasicString<char16_t>;
using StringU32 = XBasicString<char32_t>;

// Common SmallString shapes — larger inline buffer, heap fallback.
// Names mirror their typical use; pick whichever sizes the call site needs.
using NameString = BasicSmallString<char, 32>;     // identifiers, block names
using PathString = BasicSmallString<char, 192>;    // file paths, urls

// Common FixedString shapes — pure stack, no heap, asserts on overflow.
using TagString = BasicFixedString<16>;
using ShortString = BasicFixedString<32>;
}  // namespace base
