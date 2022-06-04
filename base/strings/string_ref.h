// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/check.h>
#include <base/enum_traits.h>
#include <base/numeric_limits.h>
#include <base/strings/xstring.h>
#include <base/strings/char_algorithms.h>

namespace base {

// template <typename T>
// class BasicString;

enum class StringRefFlags {
  kNone = 1 << 0,
  kIsNullTerm = 1 << 1,
  kInvalidateDeadRef = 1 << 2,
};
BASE_IMPL_ENUM_BIT_TRAITS(StringRefFlags, u32);

// plain simple string view like class
template <typename TChar>
class BasicStringRef {
 public:
  using value_type = TChar;

  BasicStringRef() = delete;

  explicit BasicStringRef(const base::BasicString<TChar>& str)
      : data_(str.c_str()),
        length_(static_cast<u32>(str.length())),
        tags_(StringRefFlags::kIsNullTerm) {
    DCHECK(str.size() <= max_size_bytes());
  }

  BasicStringRef(const BasicStringRef<TChar>& other)
      : data_(other.data_), length_(other.length_), tags_(other.tags_) {}

  BasicStringRef(const TChar* data, mem_size length)
      : data_(data), length_(static_cast<u32>(length)) {
    tags_ = base::FindNullTerminator(data, length) ? StringRefFlags::kIsNullTerm
                                                   : StringRefFlags::kNone;
  }

  BasicStringRef(const TChar* data)
      : data_(data),
        length_(
            static_cast<u32>(base::CalculateStringLength(data, max_size_bytes()))) {
    // if our length is greater than 0, it means we have hit the null barrier..., so
    // we assume for now that we may advance by one to catch the actual null
    // terminator, that would have been cut off otherwise, since the specification
    // states that .length never shall include the null terminator
    tags_ = base::FindNullTerminator(data, length_ > 0 ? length_ + 1 : 0)
                ? StringRefFlags::kIsNullTerm
                : StringRefFlags::kNone;
  }

  ~BasicStringRef() {
#if 0
    if (tags_ & StringRefFlags::kInvalidateDeadRef) {
      data_ = nullptr;
      length_ = 0;
    }
#endif
  }

  inline bool IsNullTerminated() { return tags_ & StringRefFlags::kIsNullTerm; }

  // NOTE(Vince): fixes one of my biggest pet peeves with the STL, which is the fact
  // that we cannot know if a referenced string is null terminated using
  // std::string_view
  // Prefer using this over .data()
  inline const TChar* c_str() const {
    DCHECK(tags_ & StringRefFlags::kIsNullTerm,
           "String piece is not null terminated. c_str() is therefore illegal");

    // TODO: review the impact of this..
    // tags_ |= StringRefFlags::kInvalidateDeadRef;
    return data_;
  }

  inline const TChar* data() const { return data_; }

  inline const TChar* begin() const { return data_; }
  inline const TChar* end() const { return data_ + length_; }

  // returns the size in bytes
  constexpr mem_size size_bytes() const {
    return static_cast<mem_size>(length_) * sizeof(TChar);
  }

  constexpr static mem_size max_size_bytes() {
    return mem_size(base::MinMax<u32>::max());
  }

  constexpr static mem_size max_size_characters() {
    return mem_size(base::MinMax<u32>::max()) / sizeof(TChar);
  }

  mem_size size() const { return static_cast<mem_size>(length_); }
  // returns the length in characters
  mem_size length() const { return static_cast<mem_size>(length_); }

 private:
  const TChar* data_;
  const u32 length_;
  StringRefFlags tags_;
};

// most common types
using StringRef = BasicStringRef<char>;
using StringRefW = BasicStringRef<wchar_t>;
using StringRefU8 = BasicStringRef<char8_t>;
using StringRefU16 = BasicStringRef<char16_t>;
using StringRefU32 = BasicStringRef<char32_t>;
}  // namespace base