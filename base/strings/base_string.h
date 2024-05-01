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

// TODO: small string optimization

namespace base {
template <typename T, typename TEncoding>
concept HasStringTraits = requires(T& t) {
  t.data();
  t.c_str();
  t.size();
  // Direct type constraint for character type
  // { typename T::value_type{} } -> std::same_as<TEncoding>;
};

template <typename T, class TAllocator = base::DefaultAllocator>
class BasicBaseString {
 public:
  using character_type = T;
  using allocator_type = TAllocator;
  using value_type = character_type;
  using size_type = mem_size;

  BasicBaseString() noexcept = default;

  // construct from a string
  explicit BasicBaseString(const character_type* str) { assign(str); }
  explicit BasicBaseString(const character_type* str,
                           mem_size len_in_characters) {
    assign(str, len_in_characters);
  }

  // construct from string with range
  explicit BasicBaseString(const character_type* begin,
                           const character_type* end) {
    assign(begin, end - begin);
  }

  // construct from a character array
  template <mem_size N>
  explicit BasicBaseString(const character_type (&arr)[N]) {
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
    size_ = other.size_;
    capacity_ = other.capacity_;
    // condem the other
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }
  ~BasicBaseString() { DeAllocate(); }

  // Returns a pointer to the underlying character array.
  const character_type* c_str() const noexcept { return data_; }
  character_type* data() noexcept { return data_; }

  // begin and end
  const character_type* begin() const noexcept { return data_; }
  const character_type* end() const noexcept { return data_ + size_; }
  character_type* begin() noexcept { return data_; }
  character_type* end() noexcept { return data_ + size_; }
  const character_type* cbegin() const noexcept { return data_; }
  const character_type* cend() const noexcept { return data_ + size_; }
  const character_type* rbegin() const noexcept { return data_ + size_ - 1; }
  const character_type* rend() const noexcept { return data_ - 1; }
  const character_type back() const noexcept { return data_[size_ - 1]; }

  // Returns the length of the BasicBaseString.
  mem_size size() const noexcept { return size_; }
  mem_size byte_size() const noexcept { return size_ * sizeof(character_type); }
  mem_size length() const noexcept { return size_; }
  bool empty() const noexcept { return size_ == 0; }
  bool not_empty() const noexcept { return size_ != 0; }
  mem_size capacity() const noexcept { return capacity_; }
  // Returns the maximum possible length of the BasicBaseString.
  mem_size max_size() const noexcept {
    return base::MinMax<mem_size>::max() / sizeof(character_type);
  }

  // Increases the capacity of the BasicBaseString to a value greater than or
  // equal to the given size.
  void reserve(mem_size new_capacity) {
    if (new_capacity >= capacity_) {
      Reallocate(new_capacity);
    }
  }

  // Resizes the BasicBaseString to the specified length.
  void resize(mem_size new_size) {
    if (new_size >= capacity_) {
      Reallocate(new_size);
    }
    size_ = new_size;
    data_[size_] = '\0';
  }

  // append operator
  BasicBaseString& operator+=(const BasicBaseString& other) {
    append(other.data_, other.size_);
    return *this;
  }
  BasicBaseString& operator+=(const character_type* str) {
    append(str, base::CountStringLength(str));
    return *this;
  }
  BasicBaseString& operator+=(character_type c) {
    push_back(c);
    return *this;
  }

  // Appends the given BasicBaseString to the end of this BasicBaseString.
  void append(const BasicBaseString& other) {
    append(other.data_, other.size_);
  }
  void append(const character_type* str, mem_size len_in_characters) {
    mem_size new_size = size_ + len_in_characters;
    if (new_size + 1 > capacity_) {
      Reallocate(new_size);
    }
    memcpy(data_ + size_, str, len_in_characters * sizeof(character_type));
    size_ = new_size;
    data_[size_] = '\0';
  }
  void append(const character_type* str) {
	append(str, base::CountStringLength(str));
  }

  void push_back(character_type c) {
    const auto new_size = size_ + 1;
    if (new_size >= capacity_) {
      Reallocate(new_size);
    }
    data_[size_] = c;  // append the character
    size_ = new_size;  // all allocd data is zerod, so it doesnt matter.
  }

  // assignment functions ========================================
  void assign(const character_type* str, mem_size len_in_characters) {
    if (str == nullptr || len_in_characters == 0) {
	  return;
	}
    // make sure we have enough space
    if (len_in_characters >= capacity_) {
      Reallocate(len_in_characters);
    }
    // if the new string is smaller than the old one, we can just copy it
    else if (len_in_characters < capacity_) {
	  memset(data_, 0, capacity_ * sizeof(character_type));
	}

    memcpy(data_, str, len_in_characters * sizeof(character_type));
    size_ = len_in_characters;
    data_[size_] = '\0';
  }
  void assign(const character_type* begin, const character_type* end) {
    assign(begin, end - begin);
  }
  void assign(const character_type* str) {
    assign(str, base::CountStringLength(str));
  }
  BasicBaseString& operator=(const BasicBaseString& other) {
    if (this != &other) {
      assign(other.data_, other.size_);
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
      size_ = other.size_;
      capacity_ = other.capacity_;
    }
    return *this;
  }

  // equality comparisions
  int compare(const character_type* str, mem_size len) const noexcept {
    return memcmp(data_, str, len);
  }
  int compare(size_t pos,
              size_t len,
              const BasicBaseString& str) const noexcept {
    return memcmp(data_, str.c_str(), str.size());
  }
  int compare(size_t pos,
              size_t len,
              const character_type* str) const noexcept {
    return memcmp(data_, str, len);
  }
  int compare(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str));
  }
  template <class TOther>
  int compare(const TOther& other)
    requires(base::HasStringTraits<TOther, value_type>)
  {
    return memcmp(data_, other.c_str(), other.length());
  }
  bool operator==(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str)) == 0;
  }
  bool operator!=(const character_type* str) const noexcept {
    return memcmp(data_, str, base::CountStringLength(str)) != 0;
  }
  /* bool operator==(const BasicBaseString& other) const noexcept {
        return memcmp(data_, other.data_, other.size_) == 0;
  }*/
  template <class TOther>
    requires(  //! std::same_as<TOther, BasicBaseString> &&
        HasStringTraits<TOther, value_type>)
  bool operator==(const TOther& other) const {
    return memcmp(data_, other.c_str(), other.size()) == 0;
  }

  // comparision operators
  friend bool operator!=(const BasicBaseString& lhs,
                         const BasicBaseString& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(const BasicBaseString& lhs,
                        const BasicBaseString& rhs) {
    return lhs.length() < rhs.length();
  }

  friend bool operator<=(const BasicBaseString& lhs,
                         const BasicBaseString& rhs) {
    return !(rhs < lhs);
  }

  friend bool operator>(const BasicBaseString& lhs,
                        const BasicBaseString& rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(const BasicBaseString& lhs,
                         const BasicBaseString& rhs) {
    return !(lhs < rhs);
  }

  // access operator
  character_type& operator[](mem_size index) {
    BUGCHECK(index < size_, "Index out of bounds");
    return data_[index];
  }
  const character_type& operator[](mem_size index) const {
    BUGCHECK(index < size_, "Index out of bounds");
    return data_[index];
  }
  character_type* at(mem_size index) {
    BUGCHECK(index < size_, "Index out of bounds");
    return data_ + index;
  }

  // search functions ========================================
  // Finds the last occurrence of any of the characters in the given string.
  mem_size find_last_of(const BasicBaseString& other) const {
    return find_last_of(other.data_, other.size());
  }

  mem_size find_last_of(const character_type* str, mem_size len) const {
    for (mem_size i = size_ - 1; i != static_cast<mem_size>(-1); --i) {
      for (mem_size j = 0; j < len; ++j) {
        if (data_[i] == str[j]) {
          return i;
        }
      }
    }
    return npos;
  }
  mem_size find_last_of(const character_type* s, size_t pos, size_t n) const {
    for (mem_size i = pos; i != static_cast<mem_size>(-1); --i) {
      for (mem_size j = 0; j < n; ++j) {
        if (data_[i] == s[j]) {
          return i;
        }
      }
    }
    return npos;
  }

  // replace functions ========================================
  void erase(mem_size pos, mem_size count = npos) {
    BUGCHECK(pos > size_, "Invalid position");

    if (count == npos) {
      count = size_ - pos;
    }

    BUGCHECK(pos + count > size_, "Invalid count");

    mem_size new_size = size_ - count;
    memmove(data_ + pos, data_ + pos + count,
            (size_ - pos - count) * sizeof(character_type));
    size_ = new_size;
    data_[size_] = '\0';
  }
  void erase(const character_type* begin, const character_type* end) {
    erase(begin - data_, end - begin);
  }
  void erase(const character_type* pos) { erase(pos - data_); }

  void clear() {
    size_ = 0;
    memset(data_, 0, capacity_ * sizeof(character_type));
  }

  static constexpr mem_size npos = base::MinMax<mem_size>::max();
  BasicBaseString substr(mem_size pos, mem_size count = npos) const {
    BUGCHECK(pos > size_, "Invalid position");

    if (count == npos) {
      count = size_ - pos;
    }

    BUGCHECK(pos + count > size_, "Invalid count");

    BasicBaseString substr;
    substr.assign(data_ + pos, count);
    return substr;
  }

  void shrink_to_fit() {
    if (size_ < capacity_) {
      Reallocate(
          size_);  // this aint it at the moment implement true shrink pls
      capacity_ = size_;
    }
  }

 private:
  // Allocates memory for the BasicBaseString.
  void Allocate(mem_size new_capacity_in_characters) {
    new_capacity_in_characters++;  // +1 for the nterm
    data_ = static_cast<character_type*>(TAllocator::Allocate(
        new_capacity_in_characters * sizeof(character_type)));
    memset(data_, 0, new_capacity_in_characters * sizeof(character_type));
  }

  // Deallocates the memory used by the BasicBaseString.
  void DeAllocate() {
    if (data_)
      TAllocator::Free(data_, (capacity_ * sizeof(character_type)));
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
  }

  // Reallocates the memory used by the BasicBaseString to the given capacity.
  void Reallocate(mem_size new_capacity_in_characters) {
    new_capacity_in_characters++;  // +1 for the nterm
    const mem_size old_size = size_;
    mem_size new_capacity =
        new_capacity_in_characters +
        (new_capacity_in_characters / 2);  // Use a growth factor
    character_type* new_data = static_cast<character_type*>(
        TAllocator::Allocate(new_capacity * sizeof(character_type)));
    // users are fucking stupid
    memset(new_data, 0, new_capacity * sizeof(character_type));
    // for instance if the user pushes back on an empty string
    if (data_ == nullptr) {
      data_ = new_data;
      capacity_ = new_capacity;
      return;
    }
    // move the old data to the new location
    memcpy(new_data, data_, old_size * sizeof(character_type));
    // free the old data buffer and nterm
    TAllocator::Free(data_, (capacity_ * sizeof(character_type)));

    // assign
    data_ = new_data;
    capacity_ = new_capacity;
  }

  character_type* data_ = nullptr;
  mem_size size_ = 0;
  mem_size capacity_ = 0;
};

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