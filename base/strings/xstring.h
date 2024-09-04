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

#if 0

//#define BASE_USE_STDSTRING
#ifndef BASE_USE_STDSTRING
#define FOLLY_HAS_STRING_VIEW 1
#include <base/external/folly/fbstring.h>
#define BASE_STRING(x) folly::basic_fbstring<x>
#else
#include <string>
#define BASE_STRING(x) std::basic_string<x>
#endif
#endif

#if 0
#include <string>
#include <base/strings/eqi_string.h>
#define BASE_STRING(x) base::basic_eqi_string<x>
#endif

#if 1
#include <base/strings/base_string.h>
#define BASE_STRING(x) base::BasicBaseString<x>
#endif

namespace base {
template <typename T>
using XBasicString = BASE_STRING(T);
// define our core string types.
using String = XBasicString<char>;
using StringW = XBasicString<wchar_t>;
// UTF Strings
using StringU8 = XBasicString<char8_t>;
using StringU16 = XBasicString<char16_t>;
using StringU32 = XBasicString<char32_t>;

#ifdef HAS_BASE_STRING_TRAITS
// This can lead to a saving of 8 bytes per string
using SmallString = base::BasicBaseString<char, u32>;
using SmallStringW = base::BasicBaseString<wchar_t, u32>;
using SmallStringU8 = base::BasicBaseString<char8_t, u32>;
using SmallStringU16 = base::BasicBaseString<char16_t, u32>;
using SmallStringU32 = base::BasicBaseString<char32_t, u32>;

// this is pretty useless on 64 bit systems due to alignment
using SmallestString = base::BasicBaseString<char, u16>;
using SmallestStringW = base::BasicBaseString<wchar_t, u16>;
using SmallestStringU8 = base::BasicBaseString<char8_t, u16>;
using SmallestStringU16 = base::BasicBaseString<char16_t, u16>;
using SmallestStringU32 = base::BasicBaseString<char32_t, u16>;
#endif
}  // namespace base