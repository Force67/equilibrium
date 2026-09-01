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

// std::format interop. base does its own formatting (base/strings/format.h) and
// does not use <format> itself, but a consumer that does must not be left with
// the library's default: BasicBaseString exposes begin()/end(), so C++23 range
// formatting happily prints it as ['a', 'b', ...] instead of the text, and a
// standard library without range formatting rejects it outright. Specializing
// here, next to the type, is what makes both cases behave.
//
// Define BASE_NO_STD_FORMAT to keep <format> out of the build entirely.
#if !defined(BASE_NO_STD_FORMAT) && __has_include(<format>)
#include <format>

template <typename TChar>
struct std::formatter<base::XBasicString<TChar>, TChar>
    : std::formatter<std::basic_string_view<TChar>, TChar> {
  template <typename TContext>
  auto format(const base::XBasicString<TChar>& value, TContext& context) const {
    return std::formatter<std::basic_string_view<TChar>, TChar>::format(
        std::basic_string_view<TChar>(value.data(), value.size()), context);
  }
};
#endif
