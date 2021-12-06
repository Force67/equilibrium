// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// XString implementation - our better optimized string class.
#pragma once

#include <memory>
#include <string>
#include "base/export.h"

namespace base {

/*
template <typename CharType>
class BASE_EXPORT XString {
 public:
  XString(const char* const);
  XString(const wchar_t* const);
  XString(XString& other);

 private:
  std::unique_ptr<CharType[]> data_;
  size_t length_;
};

template <typename CharType>
base::XString<CharType>::XString(const char* const) {

}
*/ 

template <typename T>
using XString = std::basic_string<T>;


using String = XString<char>;

//static_assert(sizeof(XString<char>) == sizeof(uintptr_t) * 2);

class BASE_EXPORT FixedString {
 public:
};
}  // namespace base