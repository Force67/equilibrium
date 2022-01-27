// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// XString implementation - our optimized string class.
// Note that this file has to be called 'xstring.h' intead of 'string.h' for
// backwards compat reasons with the c header.
#pragma once

#include <base/export.h>

// Note(Vince): This is a switch so you can avoid using the heavy std header
// While this isnt of much use for now as fbstring pulls in all the std headers
// anyway, its a future optimization/portability point
#define BASE_USE_STDSTRING
#ifndef BASE_USE_STDSTRING
#define FOLLY_HAS_STRING_VIEW 1
#include <base/external/folly/fbstring.h>
#define BASE_STRING(x) folly::fbstring_core<x>
#else
#include <string>
#define BASE_STRING(x) std::basic_string<x>
#endif

namespace base {
template <typename T>
using BasicString = BASE_STRING(T);
template <typename T>
using BasicStringRef = std::basic_string_view<T>;

using String = BasicString<char>;
using Wstring = BasicString<wchar_t>;

using StringRef = BasicStringRef<char>;
using WStringRef = BasicStringRef<wchar_t>;
using StringRef = BasicStringRef<char>;
}  // namespace base