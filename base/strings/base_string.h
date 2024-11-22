// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// NOTE(Vince): yet another string class, this time with a custom allocator
// and mostly for me to be able to super optimize it for my performance specific
// needs as well as to have a stable interface for everything that needs a
// string, be it on a dll boundary or not
#pragma once

// we try to avoid expensive headers.
#include <base/arch.h>
#include <base/numeric_limits.h>
#include <base/containers/container_traits.h>
#include <base/strings/char_algorithms.h>

#include <cstring>
#include <algorithm>

// TODO: small string optimization

#define HAS_BASE_STRING_TRAITS 1

namespace base {
template <typename T, typename TEncoding>
concept HasStringTraits = requires(T& t) {
  t.data();
  t.c_str();
  t.size();
  // Direct type constraint for character type
  // { typename T::value_type{} } -> std::same_as<TEncoding>;
};

template <typename TChar,
          typename TSizeType = mem_size,
          class TAllocator = base::DefaultAllocator>
class BasicBaseString {
 public:
  using character_type = TChar;
  using allocator_type = TAllocator;
  using value_type = character_type;
  using size_type = TSizeType;

  BasicBaseString() noexcept = default;

  // construct from a string
  /*implicit*/ BasicBaseString(const character_type* str) { assign(str); }
  explicit BasicBaseString(const character_type* str, size_type len_in_characters) {
    assign(str, len_in_characters);
  }

  // construct from string with range
  explicit BasicBaseString(const character_type* begin, const character_type* end) {
    assign(begin, end - begin);
  }

  // construct from a character array
  template <size_type N>
  /*implicit*/ BasicBaseString(const character_type (&arr)[N]) {
    assign(arr, N - 1);
  }

  // copy constructor
  BasicBaseString(const BasicBaseString& other) {
    if (this != &other) {
      assign(other.c_str(), other.size());
    }
  }

  template <class TOther>
    requires(  //! std::same_as<TOther, BaseString> &&
        base::HasStringTraits<TOther, value_type>)
  BasicBaseString(const TOther& other) {
    // if (this != &other) {
    assign(other.c_str(), other.size());
    //}
  }

  // move constructor
  BasicBaseString(BasicBaseString&& other) noexcept {
    data_ = other.data_;
    size_in_chars_ = other.size_in_chars_;
    cap_in_chars_ = other.cap_in_chars_;
    // condem the other
    other.data_ = nullptr;
    other.size_in_chars_ = 0;
    other.cap_in_chars_ = 0;
  }
  ~BasicBaseString() { DeAllocate(); }

  // Returns a pointer to the underlying character array.
  const character_type* c_str() const noexcept { return data_; }
  character_type* data() const noexcept { return data_; }

  // const character_type front() const noexcept { return data_[0]; }

  // begin and end
  const character_type* begin() const noexcept { return data_; }
  const character_type* end() const noexcept { return data_ + size_in_chars_; }
  character_type* begin() noexcept { return data_; }
  character_type* end() noexcept { return data_ + size_in_chars_; }
  const character_type* cbegin() const noexcept { return data_; }
  const character_type* cend() const noexcept { return data_ + size_in_chars_; }
  const character_type* rbegin() const noexcept { return data_ + size_in_chars_ - 1; }
  const character_type* rend() const noexcept { return data_ - 1; }
  const character_type back() const noexcept { return data_[size_in_chars_ - 1]; }

  // Returns the length of the BasicBaseString.
  size_type size() const noexcept { return size_in_chars_; }
  size_type byte_size() const noexcept { return size_in_chars_ * sizeof(character_type); }
  size_type length() const noexcept { return size_in_chars_; }
  bool empty() const noexcept { return size_in_chars_ == 0; }
  bool not_empty() const noexcept { return size_in_chars_ != 0; }
  size_type capacity() const noexcept { return cap_in_chars_; }
  // Returns the maximum possible length of the BasicBaseString.
  size_type max_size() const noexcept {
    return base::MinMax<size_type>::max() / sizeof(character_type);
  }

  // Increases the capacity of the BasicBaseString to a value greater than or
  // equal to the given size.
  void reserve(size_type new_capacity) {
    if (new_capacity >= cap_in_chars_) {
      Reallocate(new_capacity);
    }
  }

  // Resizes the BasicBaseString to the specified length.
  void resize(size_type new_size) {
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }
    size_in_chars_ = new_size;
    data_[size_in_chars_] = '\0';
  }

  // append operator
  BasicBaseString& operator+=(const BasicBaseString& other) {
    append(other.data_, other.size_in_chars_);
    return *this;
  }
  BasicBaseString& operator+=(const character_type* str) {
    append(str);
    return *this;
  }
  BasicBaseString& operator+=(character_type c) {
    push_back(c);
    return *this;
  }

  // Appends the given BasicBaseString to the end of this BasicBaseString.
  void append(const BasicBaseString& other) { append(other.data_, other.size_in_chars_); }
  void append(const character_type* str, size_type added_character_count) {
    size_type new_size = size_in_chars_ + added_character_count;
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }
    memcpy(&data_[size_in_chars_], str, added_character_count * sizeof(character_type));
    size_in_chars_ = new_size;
    // data_[size_in_chars_] = '\0'; no point in zeroing as we realloc zerod
    // anyway
  }
  void append(const character_type* str) {
    if (!str ||
        str[0] == '\0') {  // counting the strlen would fail if we let this through
      return;
    }
    append(str, base::CountStringLength(str));
  }
  void append(const size_type n, const character_type c) {
    size_type new_size = size_in_chars_ + n;
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }
    memset(&data_[size_in_chars_], c, n * sizeof(character_type));
    size_in_chars_ = new_size;
    data_[size_in_chars_] = '\0';
  }
  void append(const character_type single_character) {
    const size_type new_size = size_in_chars_ + 1;
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }
    data_[size_in_chars_] = single_character;
    size_in_chars_ = new_size;
    data_[size_in_chars_] = '\0';
  }

  void push_back(character_type c) {
    const auto new_size = size_in_chars_ + 1;
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }
    data_[size_in_chars_] = c;  // append the character
    size_in_chars_ = new_size;  // all allocd data is zerod, so it doesnt matter.
  }

  // assignment functions ========================================
  void assign(const character_type* str, size_type len_in_characters) {
    if (str == nullptr || len_in_characters == 0) {
      return;
    }
    // make sure we have enough space
    if (len_in_characters >= cap_in_chars_) {
      Reallocate(len_in_characters);
    }
    // if the new string is smaller than the old one, we can just copy it
    else if (len_in_characters < cap_in_chars_) {
      memset(data_, 0, cap_in_chars_ * sizeof(character_type));
    }

    memcpy(data_, str, len_in_characters * sizeof(character_type));
    size_in_chars_ = len_in_characters;
    data_[size_in_chars_] = '\0';
  }
  void assign(const character_type* begin, const character_type* end) {
    assign(begin, end - begin);
  }
  void assign(const character_type* str) { assign(str, base::CountStringLength(str)); }
  BasicBaseString& operator=(const BasicBaseString& other) {
    if (this != &other) {
      assign(other.data_, other.size_in_chars_);
    }
    return *this;
  }
  BasicBaseString& operator=(const character_type* str) {
    assign(str, base::CountStringLength(str));
    return *this;
  }

  // move assignment ========================================
  BasicBaseString& operator=(BasicBaseString&& other) noexcept {
    if (this != &other) {
      data_ = other.data_;
      size_in_chars_ = other.size_in_chars_;
      cap_in_chars_ = other.cap_in_chars_;
      // condem the other
      other.data_ = nullptr;
      other.size_in_chars_ = 0;
      other.cap_in_chars_ = 0;
    }
    return *this;
  }

  // equality comparisions
  int compare(const character_type* str, size_type len) const noexcept {
    return memcmp(data_, str, len);
  }
  int compare(size_type pos, size_type len, const BasicBaseString& str) const noexcept {
    if (pos == 0 && len == 0)
      return 0;

    // Check if the requested substring is within the bounds of the current
    // string
    if (pos > size_in_chars_) {
      return str.empty() ? 0 : -1;  // If the substring is beyond the end of the
                                    // string, compare with an empty string
    }

    size_type rlen =
        std::min(len, size_in_chars_ - pos);  // Length of the substring to compare

    // Compare the substring with the provided string
    int result = memcmp(data_ + pos, str.data(), std::min(rlen, str.size()));

    // If the substrings are equal, compare the remaining characters
    if (result == 0) {
      if (rlen < str.size()) {
        result = -1;
      } else if (rlen > str.size()) {
        result = 1;
      } else {
        result = 0;
      }
    }
    return result;
  }
  int compare(size_type pos, size_type len, const character_type* str) const noexcept {
    // Check if the requested substring is within the bounds of the current
    // string
    if (pos > size_in_chars_) {
      // If the substring is beyond the end of the string, compare with a
      // null-terminated string
      int result = memcmp("", str, 1);
      return result == 0 ? 0 : -1;
    }

    size_type rlen =
        std::min(len, size_in_chars_ - pos);  // Length of the substring to compare

    // Compare the substring with the provided string
    int result = memcmp(&data_[pos], str, rlen * sizeof(character_type));

    // If the substrings are equal, compare the remaining characters
    if (result == 0) {
      size_type str_len = 0;
      while (str[str_len] != 0) {
        str_len++;
      }
      if (rlen < str_len) {
        result = -1;
      } else if (rlen > str_len) {
        result = 1;
      } else {
        result = 0;
      }
    }
    return result;
  }
  int compare(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str) * sizeof(character_type));
  }
  template <class TOther>
  int compare(const TOther& other)
    requires(base::HasStringTraits<TOther, value_type>)
  {
    return memcmp(data_, other.c_str(), other.length() * sizeof(character_type));
  }
  bool operator==(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str) * sizeof(character_type)) == 0;
  }
  bool operator!=(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str) * sizeof(character_type)) != 0;
  }
  /* bool operator==(const BasicBaseString& other) const noexcept {
        return memcmp(data_, other.data_, other.size_in_chars_) == 0;
  }*/
  template <class TOther>
    requires(  //! std::same_as<TOther, BasicBaseString> &&
        HasStringTraits<TOther, value_type>)
  bool operator==(const TOther& other) const {
    const bool result =
        memcmp(data_, other.c_str(), other.size() * sizeof(character_type)) == 0;
    return result;
  }

  // comparision operators
  friend bool operator!=(const BasicBaseString& lhs, const BasicBaseString& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const BasicBaseString& lhs, const BasicBaseString& rhs) {
    return lhs.length() < rhs.length();
  }

  friend bool operator<=(const BasicBaseString& lhs, const BasicBaseString& rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const BasicBaseString& lhs, const BasicBaseString& rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const BasicBaseString& lhs, const BasicBaseString& rhs) {
    return !(lhs < rhs);
  }

  // access operator
  character_type& operator[](size_type index) {
    BUGCHECK(index < size_in_chars_, "Index out of bounds");
    return data_[index];
  }
  const character_type& operator[](size_type index) const {
    BUGCHECK(index < size_in_chars_, "Index out of bounds");
    return data_[index];
  }
  character_type& at(size_type index) {
    BUGCHECK(index < size_in_chars_, "Index out of bounds");
    return data_[index];
  }

  character_type* at_if(size_type index) {
    if (index < size_in_chars_) {
      return data_ + index;
    }
    return nullptr;
  }

  // search functions ========================================
  // Finds the last occurrence of any of the characters in the given string.
  size_type find(const character_type c) {
    for (size_type i = 0; i < size_in_chars_; ++i) {
      if (data_[i] == c) {
        return i;
      }
    }
    return npos;
  }

  size_type find_last_of(const BasicBaseString& other) const {
    return find_last_of(other.data_, other.size());
  }

  size_type find_last_of(const character_type* str, size_type len) const {
    for (size_type i = size_in_chars_ - 1; i != static_cast<size_type>(-1); --i) {
      for (size_type j = 0; j < len; ++j) {
        if (data_[i] == str[j]) {
          return i;
        }
      }
    }
    return npos;
  }
  /*Position of the last character in the string to be considered in the search.
Any value greater than, or equal to, the string length (including string::npos)
means that the entire string is searched. Note: The first character is denoted
by a value of 0 (not 1).*/
  size_type find_last_of(const character_type* s, size_type pos, size_type n) const {
    if (pos >= size_in_chars_) {
      pos = size_in_chars_ - 1;
    }
    for (size_type i = pos; i != static_cast<size_type>(-1); --i) {
      for (size_type j = 0; j < n; ++j) {
        if (data_[i] == s[j]) {
          return i;
        }
      }
    }
    return npos;
  }

  // replace functions ========================================
  void erase(mem_size pos_in_characters, size_type count = npos) {
    // Check if the position is within the valid range
    BUGCHECK(pos_in_characters < size_in_chars_, "Invalid position");

    // Adjust the count if it's set to npos
    if (count == npos) {
      count = size_in_chars_ - pos_in_characters;
    }

    // Check if the count is within the valid range
    BUGCHECK(count <= size_in_chars_ - pos_in_characters, "Invalid count");

    // Shift the characters after the deleted region
    memmove(reinterpret_cast<byte*>(data_) + (pos_in_characters * sizeof(character_type)),
            reinterpret_cast<byte*>(data_) +
                ((pos_in_characters + count) * sizeof(character_type)),
            (size_in_chars_ - pos_in_characters - count) * sizeof(character_type));

    // Update the size
    size_in_chars_ -= count;
    data_[size_in_chars_] = '\0';  // Ensure null termination
  }
  void erase(const character_type* first, const character_type* last) {
    erase(first - data_, last - first);
  }
  void erase(const character_type* pos) { erase(pos - data_, 1); }

  void remove_suffix(mem_size n) {
    BUGCHECK(n <= size_in_chars_, "Invalid count");
    size_in_chars_ -= n;
    data_[size_in_chars_] = '\0';  // Ensure null termination
  }

  void clear() {
    size_in_chars_ = 0;
    memset(data_, 0, cap_in_chars_ * sizeof(character_type));
  }

  static constexpr size_type npos = base::MinMax<size_type>::max();
  BasicBaseString substr(size_type pos, size_type count = npos) const {
    BUGCHECK(pos > size_in_chars_, "Invalid position");

    if (count == npos) {
      count = size_in_chars_ - pos;
    }

    BUGCHECK(pos + count > size_in_chars_, "Invalid count");

    BasicBaseString substr;
    substr.assign(data_ + pos, count);
    return substr;
  }

  void shrink_to_fit() {
    if (size_in_chars_ < cap_in_chars_) {
      Reallocate(size_in_chars_);  // this aint it at the moment implement true
                                   // shrink pls
      cap_in_chars_ = size_in_chars_;
    }
  }

  // insert functions ====
  void insert(mem_size pos, size_type n, character_type character) {
    // Check if the position is within the valid range
    BUGCHECK(pos <= size_in_chars_, "Invalid position");

    // Adjust the size of the string to make room for the new characters
    size_type new_size = size_in_chars_ + n;
    if (new_size >= cap_in_chars_) {
      Reallocate(new_size);
    }

    // Shift the characters after the insertion point to make room
    memmove(data_ + pos + n, data_ + pos,
            (size_in_chars_ - pos) * sizeof(character_type));

    // Insert the new characters
    memset(data_ + pos, character, n * sizeof(character_type));

    // Update the size of the string
    size_in_chars_ = new_size;
    data_[size_in_chars_] = '\0';  // Ensure null termination
  }

 private:
  // Allocates memory for the BasicBaseString.
  void Allocate(size_type new_cap_in_chars_in_characters) {
    new_cap_in_chars_in_characters++;  // +1 for the nterm
    data_ = static_cast<character_type*>(
        TAllocator::Allocate(new_cap_in_chars_in_characters * sizeof(character_type)));
    memset(data_, 0, new_cap_in_chars_in_characters * sizeof(character_type));
  }

  // Deallocates the memory used by the BasicBaseString.
  void DeAllocate() {
    if (data_)
      TAllocator::Free(data_, (cap_in_chars_ * sizeof(character_type)));
    data_ = nullptr;
    size_in_chars_ = 0;
    cap_in_chars_ = 0;
  }

  // Reallocates the memory used by the BasicBaseString to the given capacity.
  void Reallocate(size_type new_cap_in_chars_in_characters) {
    new_cap_in_chars_in_characters++;  // +1 for the nterm
    const size_type old_size = size_in_chars_;
    size_type new_capacity = new_cap_in_chars_in_characters +
                             (new_cap_in_chars_in_characters / 2);  // Use a growth factor
    character_type* new_data = static_cast<character_type*>(
        TAllocator::Allocate(new_capacity * sizeof(character_type)));
    // users are fucking stupid
    memset(new_data, 0, new_capacity * sizeof(character_type));
    // for instance if the user pushes back on an empty string
    if (data_ == nullptr) {
      data_ = new_data;
      cap_in_chars_ = new_capacity;
      return;
    }
    // move the old data to the new location
    memcpy(new_data, data_, old_size * sizeof(character_type));
    // free the old data buffer and nterm
    TAllocator::Free(data_, (cap_in_chars_ * sizeof(character_type)));

    // assign
    data_ = new_data;
    cap_in_chars_ = new_capacity;
  }

  character_type* data_ = nullptr;
  size_type size_in_chars_ = 0;
  size_type cap_in_chars_ = 0;
};

template <typename CharT, typename Alloc>
BasicBaseString<CharT, Alloc> operator+(const BasicBaseString<CharT, Alloc>& lhs,
                                        const BasicBaseString<CharT, Alloc>& rhs) {
  BasicBaseString<CharT, Alloc> result(lhs);
  result += rhs;
  return result;
}

template <typename CharT, typename Alloc>
BasicBaseString<CharT, Alloc> operator+(const BasicBaseString<CharT, Alloc>& lhs,
                                        const CharT* rhs) {
  BasicBaseString<CharT, Alloc> result(lhs);
  result += rhs;
  return result;
}

template <typename CharT, typename Alloc>
BasicBaseString<CharT, Alloc> operator+(const CharT* lhs,
                                        const BasicBaseString<CharT, Alloc>& rhs) {
  BasicBaseString<CharT, Alloc> result(lhs);
  result += rhs;
  return result;
}

#if 0
// define our core string types.
using BaseString = BasicBaseString<char>;
using BaseStringW = BasicBaseString<wchar_t>;
// UTF Strings
using BaseStringU8 = BasicBaseString<char8_t>;
using BaseStringU16 = BasicBaseString<char16_t>;
using BaseStringU32 = BasicBaseString<char32_t>;
#endif
}  // namespace base
