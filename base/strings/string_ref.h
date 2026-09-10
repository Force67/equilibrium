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

  // default ctor: empty ref pointing to a static empty string
  constexpr BasicStringRef()
      : data_(s_empty_), length_(0), tags_(StringRefFlags::kIsNullTerm) {}

  // construct from base::String<T>
  template <class TOther>
    requires(base::HasStringTraits<TOther, value_type>)
  BasicStringRef(const TOther& str)
      : data_(str.c_str()),
        length_(static_cast<u32>(str.length())),
        tags_(StringRefFlags::kIsNullTerm) {
    BASE_DCHECK(str.size() <= max_size_characters());
  }

  // construct from any other non-owning character range (std::string_view).
  // A foreign view makes no null-termination promise, so neither do we.
  template <class TView>
    requires(base::StringViewLike<TView, value_type> &&
             !base::HasStringTraits<TView, value_type> &&
             !base::is_same_v<TView, BasicStringRef>)
  BasicStringRef(const TView& view)
      : data_(view.data()),
        length_(static_cast<u32>(view.size())),
        tags_(StringRefFlags::kNone) {
    BASE_DCHECK(view.size() <= max_size_characters());
  }

  // Implicit conversion out to any view constructible from (pointer, length),
  // so a StringRef reaches an API spelled in std::string_view.
  template <class TView>
    requires(base::ConstructibleView<TView, value_type> &&
             !base::is_same_v<TView, BasicStringRef>)
  constexpr operator TView() const {
    return TView(data_, length_);
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
  constexpr BasicStringRef(const TChar* data, mem_size length, bool is_null_terminated)
      : data_(data),
        length_(static_cast<u32>(length)),
        tags_(is_null_terminated ? StringRefFlags::kIsNullTerm : StringRefFlags::kNone) {}

  // construct from raw string
  constexpr BasicStringRef(const TChar* data)
      : data_(data),
        length_(static_cast<u32>(base::CountStringLength(data, max_size_bytes()))) {
    // if our length is greater than 0, it means we have hit the null
    // barrier..., so we assume for now that we may advance by one to catch the
    // actual null terminator, that would have been cut off otherwise, since the
    // specification states that .length never shall include the null terminator
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

  // move assignment
  BasicStringRef<TChar>& operator=(BasicStringRef<TChar>&& other) noexcept {
    if (this != &other) {
      data_ = other.data_;
      length_ = other.length_;
      tags_ = other.tags_;
    }
    return *this;
  }
  // copy assignment
  BasicStringRef<TChar>& operator=(const BasicStringRef<TChar>& other) noexcept {
    if (this != &other) {
      data_ = other.data_;
      length_ = other.length_;
      tags_ = other.tags_;
    }
    return *this;
  }

  bool compare(const TChar* rhs, mem_size character_count) const {
    return memcmp(data_, rhs, character_count * sizeof(TChar)) == 0;
  }

  bool compare_at(const mem_size self_offset_in_bytes,
                  const TChar* rhs,
                  mem_size character_count) const {
    return memcmp(data_ + self_offset_in_bytes, rhs, character_count * sizeof(TChar)) ==
           0;
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
    const mem_size min_len = lhs.length_ < rhs.length_ ? lhs.length_ : rhs.length_;
    int result = memcmp(lhs.data_, rhs.data_, min_len * sizeof(TChar));
    if (result != 0)
      return result < 0;
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
  static inline BasicStringRef<TChar> null_ref() { return BasicStringRef<TChar>(); }

  inline bool IsNullTerminated() const { return tags_ & StringRefFlags::kIsNullTerm; }

  // Unlike std::string_view, this reports whether the referenced string is
  // null terminated. Prefer over .data().
  inline const TChar* c_str() const {
    BASE_BUGCHECK(tags_ & StringRefFlags::kIsNullTerm,
                  "String piece is not null terminated. c_str() is therefore illegal");

    // TODO: review impact of tagging kInvalidateDeadRef here.
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

  // individual character search
  constexpr mem_size find(const TChar s, mem_size pos) const {
    return base::StringSearch(data_, length(), &s, pos, 1);
  }

  constexpr mem_size find(const TChar s) const {
    return base::StringSearch(data_, length(), &s, 0, 1);
  }

  constexpr mem_size find(const TChar* s) const {
    return base::StringSearch(data_, length(), s, 0, base::CountStringLength(s));
  }

  mem_size find(const BasicStringRef& s, mem_size pos = 0) const {
    return base::StringSearch(data_, length(), s.data(), pos, s.size());
  }

  bool contains(TChar c) const { return find(c) != npos; }
  bool contains(const TChar* s) const { return find(s) != npos; }

  const TChar& back() const {
    BASE_BUGCHECK(length_ > 0, "Cannot access .back() of an empty StringRef");
    return data_[length_ - 1];
  }

  const TChar& front() const {
    BASE_BUGCHECK(length_ > 0, "Cannot access .front() of an empty StringRef");
    return data_[0];
  }

  // Shrinks the view from the right. The result is no longer known to be null
  // terminated, so the flag is dropped.
  void remove_suffix(mem_size n) {
    BASE_BUGCHECK(n <= length_, "remove_suffix past the start of the StringRef");
    length_ -= static_cast<u32>(n);
    tags_ = StringRefFlags::kNone;
  }

  // Shrinks the view from the left. Still points into the same buffer, so a
  // null-terminated ref stays null terminated.
  void remove_prefix(mem_size n) {
    BASE_BUGCHECK(n <= length_, "remove_prefix past the end of the StringRef");
    data_ += n;
    length_ -= static_cast<u32>(n);
  }

  // Last occurrence at or before `pos`.
  mem_size rfind(TChar c, mem_size pos = npos) const {
    if (length_ == 0)
      return npos;
    mem_size i = (pos == npos || pos >= length_) ? length_ - 1 : pos;
    for (;; --i) {
      if (data_[i] == c)
        return i;
      if (i == 0)
        return npos;
    }
  }

  mem_size rfind(const TChar* s, mem_size pos = npos) const {
    if (!s)
      return npos;
    const mem_size s_len = base::CountStringLength(s);
    if (s_len == 0)
      return pos < length_ ? pos : length_;
    if (s_len > length_)
      return npos;
    mem_size i = (pos == npos || pos > length_ - s_len) ? length_ - s_len : pos;
    for (;; --i) {
      if (memcmp(data_ + i, s, s_len * sizeof(TChar)) == 0)
        return i;
      if (i == 0)
        return npos;
    }
  }

  // Last position at or before `pos` holding a character in `set`.
  mem_size find_last_of(TChar c, mem_size pos = npos) const { return rfind(c, pos); }

  mem_size find_last_of(const TChar* set, mem_size pos = npos) const {
    if (!set || length_ == 0)
      return npos;
    const mem_size set_len = base::CountStringLength(set);
    mem_size i = (pos == npos || pos >= length_) ? length_ - 1 : pos;
    for (;; --i) {
      for (mem_size k = 0; k < set_len; ++k) {
        if (data_[i] == set[k])
          return i;
      }
      if (i == 0)
        return npos;
    }
  }

  mem_size find_first_of(const TChar* set, mem_size pos = 0) const {
    if (!set)
      return npos;
    const mem_size set_len = base::CountStringLength(set);
    for (mem_size i = pos; i < length_; ++i) {
      for (mem_size k = 0; k < set_len; ++k) {
        if (data_[i] == set[k])
          return i;
      }
    }
    return npos;
  }

  // Three-way comparison of the [pos, pos + count) slice against another ref,
  // matching std::string_view::compare's positional overload.
  int compare(mem_size pos, mem_size count, const BasicStringRef& other) const {
    return subslice(pos, count).compare_to(other);
  }

  int compare_to(const BasicStringRef& other) const {
    const mem_size shortest = length_ < other.length_ ? length_ : other.length_;
    if (shortest > 0) {
      const int diff = memcmp(data_, other.data_, shortest * sizeof(TChar));
      if (diff != 0)
        return diff;
    }
    if (length_ == other.length_)
      return 0;
    return length_ < other.length_ ? -1 : 1;
  }

  bool starts_with(TChar c) const { return length_ > 0 && data_[0] == c; }

  bool starts_with(const TChar* s) const {
    if (!s)
      return false;
    const mem_size s_len = base::CountStringLength(s);
    if (s_len > length_)
      return false;
    return memcmp(data_, s, s_len * sizeof(TChar)) == 0;
  }

  bool starts_with(const BasicStringRef& s) const {
    if (s.size() > length_)
      return false;
    return memcmp(data_, s.data(), s.size() * sizeof(TChar)) == 0;
  }

  bool ends_with(TChar c) const { return length_ > 0 && data_[length_ - 1] == c; }

  bool ends_with(const TChar* s) const {
    if (!s)
      return false;
    const mem_size s_len = base::CountStringLength(s);
    if (s_len > length_)
      return false;
    return memcmp(data_ + (length_ - s_len), s, s_len * sizeof(TChar)) == 0;
  }

  bool ends_with(const BasicStringRef& s) const {
    if (s.size() > length_)
      return false;
    return memcmp(data_ + (length_ - s.size()), s.data(), s.size() * sizeof(TChar)) == 0;
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
    BASE_BUGCHECK(offset < length_, "Index out of bounds");
    return data_[offset];
  }

  // A view of the same buffer, like std::string_view::substr. It must not
  // allocate: a StringRef bound to an owning temporary would dangle, which is
  // exactly the trap this used to set. Use to_string() for an owning copy.
  BasicStringRef<TChar> substr(mem_size pos = 0, mem_size count = npos) const {
    BASE_BUGCHECK(pos <= length_, "Position is out of bounds");
    return subslice(pos, count);
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
  static constexpr TChar s_empty_[1] = {0};
  const TChar* data_;
  u32 length_;
  StringRefFlags tags_;
};

// most common types, the U denotes utf-ness in order to adhere somewhat to the
// stl naming conventions
using StringRef = BasicStringRef<char>;
using StringRefW = BasicStringRef<wchar_t>;
using StringRefU8 = BasicStringRef<char8_t>;
using StringRefU16 = BasicStringRef<char16_t>;
using StringRefU32 = BasicStringRef<char32_t>;

// construction helpers
// The declaration of a literal operator shall have a
// parameter-declaration-clause equivalent to one of the following: const char*
// const char*, std::size_t const wchar_t*, std::size_t const char16_t*,
// std::size_t const char32_t*, std::size_t
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

// std::format interop, for the same reason as base::String: see the note at the
// bottom of xstring.h.
#if !defined(BASE_NO_STD_FORMAT) && __has_include(<format>)
#include <format>

template <typename TChar>
struct std::formatter<base::BasicStringRef<TChar>, TChar>
    : std::formatter<std::basic_string_view<TChar>, TChar> {
  template <typename TContext>
  auto format(const base::BasicStringRef<TChar>& value, TContext& context) const {
    return std::formatter<std::basic_string_view<TChar>, TChar>::format(
        std::basic_string_view<TChar>(value.data(), value.size()), context);
  }
};
#endif
