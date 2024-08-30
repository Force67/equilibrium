// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstring>  // for memcmp
#include <base/check.h>
#include <base/enum_traits.h>
#include <base/numeric_limits.h>
#include <base/strings/xstring.h>

#include <base/strings/string_search.h>
#include <base/strings/char_algorithms.h>

#include <base/hashing/fnv1a.h>
#include <base/strings/base_string.h>

namespace base {

// template <typename T>
// class BasicString;

enum class StringRefFlags {
  kNone = 1 << 0,
  kIsNullTerm = 1 << 1,
  kInvalidateDeadRef = 1 << 2,
};
BASE_IMPL_ENUM_BIT_TRAITS(StringRefFlags, u32)

// plain simple string view like class
template <typename TChar>
class BasicStringRef {
 public:
  using value_type = TChar;
  static constexpr mem_size npos = kStringNotFoundPos;

  // disable default ctor
  BasicStringRef() = delete;

  // construct from base::String<T>
  template <class TOther>
    requires(  //! std::same_as<TOther, BaseString> &&
                base::HasStringTraits<TOther, value_type>)
  BasicStringRef(const TOther & str)
      : data_(str.c_str()),
        length_(static_cast<u32>(str.length())),
        tags_(StringRefFlags::kIsNullTerm) {
    DCHECK(str.size() <= max_size_characters());
  }

  // construct from other
  constexpr BasicStringRef(const BasicStringRef<TChar>& other)
      : data_(other.data_), length_(other.length_), tags_(other.tags_) {}

  // construct from TChar, length
  BasicStringRef(const TChar* data, mem_size length)
      : data_(data), length_(static_cast<u32>(length)) {
    tags_ = base::FindNullTerminator(data, length) ? StringRefFlags::kIsNullTerm
                                                   : StringRefFlags::kNone;
  }

  // construct at compiletime
  constexpr BasicStringRef(const TChar* data,
                           mem_size length,
                           bool is_null_terminated)
      : data_(data),
        length_(static_cast<u32>(length)),
        tags_(is_null_terminated ? StringRefFlags::kIsNullTerm
                                 : StringRefFlags::kNone) {}

  // construct from raw string
  constexpr BasicStringRef(const TChar* data)
      : data_(data),
        length_(static_cast<u32>(base::CountStringLength(data, max_size_bytes()))) {
    // if our length is greater than 0, it means we have hit the null barrier..., so
    // we assume for now that we may advance by one to catch the actual null
    // terminator, that would have been cut off otherwise, since the specification
    // states that .length never shall include the null terminator
    tags_ = base::FindNullTerminator(data, length_ > 0 ? length_ + 1 : 0)
                ? StringRefFlags::kIsNullTerm
                : StringRefFlags::kNone;
  }

  constexpr ~BasicStringRef() {
#if 0
    if (tags_ & StringRefFlags::kInvalidateDeadRef) {
      data_ = nullptr;
      length_ = 0;
    }
#endif
  }

  BasicStringRef<TChar>& operator=(BasicStringRef<TChar>&& other) noexcept {
    if (this != &other) {
      // Transfer ownership
      // havent got time for this bs..
      memcpy(this, &other, sizeof(BasicStringRef<TChar>));

    }
    return *this;
  }

  bool compare(const TChar* rhs, mem_size character_count) const {
    return memcmp(data_, rhs, character_count * sizeof(TChar)) == 0;
  }

  bool compare_at(const mem_size self_offset_in_bytes,
                  const TChar* rhs,
                  mem_size character_count) const {
    return memcmp(data_ + self_offset_in_bytes, rhs,
                  character_count * sizeof(TChar)) == 0;
  }

  // Comparison operators
  friend bool operator==(const BasicStringRef<TChar>& lhs,
                         const BasicStringRef<TChar>& rhs) {
    if (lhs.length_ != rhs.length_) {
      return false;
    }
    return memcmp(lhs.data_, rhs.data_, lhs.length_ * sizeof(TChar)) == 0;
  }

  friend bool operator!=(const BasicStringRef<TChar>& lhs,
                         const BasicStringRef<TChar>& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const BasicStringRef<TChar>& lhs,
                        const BasicStringRef<TChar>& rhs) {
    return lhs.length_ < rhs.length_;
  }

  friend bool operator<=(const BasicStringRef<TChar>& lhs,
                         const BasicStringRef<TChar>& rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const BasicStringRef<TChar>& lhs,
                        const BasicStringRef<TChar>& rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const BasicStringRef<TChar>& lhs,
                         const BasicStringRef<TChar>& rhs) {
    return !(lhs < rhs);
  }

  // Use this in the rare case where you might have an empty string_ref;
  static constexpr inline BasicStringRef<TChar> null_ref() {
    // thanks to constinit we can ensure no ugly c++ guard is generated around this,
    // so while not pretty, this is OK
    static constinit TChar null_array[] = {0};
    static constinit BasicStringRef<TChar> null_ref{null_array, 0,
                                             false /*disallow .c_str()*/};
    return null_ref;
  }

  inline bool IsNullTerminated() const {
    return tags_ & StringRefFlags::kIsNullTerm;
  }

  // NOTE(Vince): fixes one of my biggest pet peeves with the STL, which is the fact
  // that we cannot know if a referenced string is null terminated using
  // std::string_view
  // Prefer using this over .data()
  inline const TChar* c_str() const {
    BUGCHECK(tags_ & StringRefFlags::kIsNullTerm,
             "String piece is not null terminated. c_str() is therefore illegal");

    // TODO: review the impact of this..
    // tags_ |= StringRefFlags::kInvalidateDeadRef;
    return data_;
  }

  inline const TChar* data() const { return data_; }

  inline const TChar* begin() const { return data_; }
  inline const TChar* end() const { return &data_[length_]; }

  constexpr static mem_size max_size_bytes() {
    return mem_size(base::MinMax<u32>::max());
  }

  constexpr static mem_size max_size_characters() {
    return mem_size(base::MinMax<u32>::max()) / sizeof(TChar);
  }

  mem_size size() const { return static_cast<mem_size>(length_); }
  // returns the length in characters
  mem_size length() const { return static_cast<mem_size>(length_); }

  bool empty() const { return length_ == 0; }

  constexpr mem_size find(const TChar* s, mem_size pos, mem_size count) const {
    return base::StringSearch(data_, length(), s, pos, count);
  }

  constexpr mem_size find_first_not_of(const TChar* s,
                                       mem_size pos,
                                       mem_size count) const {
    return base::StringSearchNotOf(data_, length(), s, pos, count);
  }

  constexpr mem_size find_last_not_of(const TChar* s, mem_size count) const {
    return base::StringSearchLastNotOf(data_, length(), s, count);
  }

#if 0
  constexpr mem_size find_last_of(const TChar* s, mem_size pos = npos) const {
    return base::FindLastOf(data_, length(), s, pos,
                            base::CalculateStringLength(s, length_));
  }
#endif

  const TChar operator[](mem_size offset) const {
    if (offset >= length_)
        DEBUG_TRAP;
    BUGCHECK(offset < length_, "Index out of bounds");
    return data_[offset];
  }

  base::XBasicString<TChar> substr(mem_size pos = 0, mem_size count = npos) const {
    BUGCHECK(pos < length_, "Position is out of bounds");
    if (count > length_ - pos) {
      count = length_ - pos;
    }
    return base::XBasicString<TChar>(&data_[pos], count);
  }

  BasicStringRef<TChar> subslice(mem_size pos = 0, mem_size count = npos) const {
    // Clamp pos to the length of the string.
    if (pos > length_) {
      pos = length_;
    }
    // Clamp count to the difference of the length of the string and pos.
    if (count > length_ - pos) {
      count = length_ - pos;
    }
    return BasicStringRef<TChar>(&data_[pos], count);
  }

  base::XBasicString<TChar> to_string() const {
	return base::XBasicString<TChar>(data_, length_);
  }

 private:
  const TChar* data_;
  const u32 length_;
  StringRefFlags tags_;
};

// most common types, the U denotes utf-ness in order to adhere somewhat to the stl
// naming conventions
using StringRef = BasicStringRef<char>;
using StringRefW = BasicStringRef<wchar_t>;
using StringRefU8 = BasicStringRef<char8_t>;
using StringRefU16 = BasicStringRef<char16_t>;
using StringRefU32 = BasicStringRef<char32_t>;

// construction helpers
// The declaration of a literal operator shall have a parameter-declaration-clause
// equivalent to one of the following: const char* const char*, std::size_t const
// wchar_t*, std::size_t const char16_t*, std::size_t const char32_t*, std::size_t
inline base::StringRef operator""_s(const char* s) {
  return base::StringRef(s);
}
inline base::StringRef operator""_s(const char* s, size_t length) {
  return base::StringRef(s, length);
}
inline base::StringRefW operator""_s(const wchar_t* s, size_t length) {
  return base::StringRefW(s, length);
}
inline base::StringRefU8 operator""_s(const char8_t* s, size_t length) {
  return base::StringRefU8(s, length);
}
inline base::StringRefU16 operator""_s(const char16_t* s, size_t length) {
  return base::StringRefU16(s, length);
}
inline base::StringRefU32 operator""_s(const char32_t* s, size_t length) {
  return base::StringRefU32(s, length);
}

template <typename T>
base::XBasicString<T> MakeStringCopy(const base::BasicStringRef<T> slice,
                                    bool no_nterm = false) {
  base::XBasicString<T> strong(slice.data(), slice.length() + (!no_nterm ? 1 : 0));
  if (!no_nterm)
    strong[slice.length()] = 0;
  return strong;
}
}  // namespace base

namespace std {
template <typename TChar>
struct hash<base::BasicStringRef<TChar>> {
  std::size_t operator()(const base::BasicStringRef<TChar>& str) const {
    // Use a hash function to compute the hash value for the string.
    // Here is an example implementation using the std::hash function:
    return base::FNV1a32(str.data());
  }
};
}  // namespace std