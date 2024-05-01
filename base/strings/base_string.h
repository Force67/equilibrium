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
  { typename T::value_type{} } -> std::same_as<TEncoding>;
};

template <typename T, class TAllocator = base::DefaultAllocator>
class BaseString {
 public:
  using character_type = T;
  using allocator_type = TAllocator;
  using value_type = character_type;

  BaseString() noexcept = default;

  explicit BaseString(const character_type* str) { assign(str); }
  explicit BaseString(const character_type* str, mem_size len_in_characters) {
    assign(str, len_in_characters);
  }

  template <mem_size N>
  explicit BaseString(const character_type (&arr)[N]) {
    assign(arr, N - 1);
  }

  BaseString(const BaseString& other) { assign(other.data_, other.size_); }

  BaseString(BaseString&& other) noexcept {
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
  }
  ~BaseString() { deallocate(); }

  BaseString& operator=(const BaseString& other) {
    if (this != &other) {
      assign(other.data_, other.size_);
    }
    return *this;
  }
  BaseString& operator=(BaseString&& other) noexcept {
    if (this != &other) {
      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;
    }
    return *this;
  }

  // Returns a pointer to the underlying character array.
  const character_type* c_str() const noexcept { return data_; }
  const character_type* data() const noexcept { return data_; }

  // Returns the length of the BaseString.
  mem_size size() const noexcept { return size_; }
  mem_size byte_size() const noexcept { return size_ * sizeof(character_type); }
  mem_size length() const noexcept { return size_; }
  mem_size capacity() const noexcept { return capacity_; }
  // Returns the maximum possible length of the BaseString.
  mem_size max_size() const noexcept {
    return std::numeric_limits<mem_size>::max() / sizeof(character_type);
  }

  // Increases the capacity of the BaseString to a value greater than or equal
  // to the given size.
  void reserve(mem_size new_capacity) {
    if (new_capacity > capacity_) {
      reallocate(new_capacity);
    }
  }

  // Resizes the BaseString to the specified length.
  void resize(mem_size new_size) {
    if (new_size > capacity_) {
      reallocate(new_size);
    }
    size_ = new_size;
    data_[size_] = '\0';
  }

  // append operator
  BaseString& operator+=(const BaseString& other) {
    append(other.data_, other.size_);
    return *this;
  }
  BaseString& operator+=(const character_type* str) {
    append(str, base::CountStringLength(str));
    return *this;
  }

  // Appends the given BaseString to the end of this BaseString.
  void append(const BaseString& other) { append(other.data_, other.size_); }
  void append(const character_type* str, mem_size len_in_characters) {
    mem_size new_size = size_ + len_in_characters;
    if (new_size > capacity_) {
      reallocate(new_size + 1);  // takes charactersize
    }
    memcpy(data_ + size_, str, len_in_characters * sizeof(character_type));
    size_ = new_size;
    data_[size_] = '\0';
  }

  // Replaces the contents of the BaseString with the given character array.
  void assign(const character_type* str, mem_size len_in_characters) {
    deallocate();
    allocate(len_in_characters + 1);
    memcpy(data_, str, len_in_characters * sizeof(character_type));
    capacity_ = len_in_characters;
    size_ = len_in_characters;
    data_[size_] = '\0';
  }

  // Replaces the contents of the BaseString with the given C-style BaseString.
  void assign(const character_type* str) {
    assign(str, base::CountStringLength(str));
  }

  // equality comparisions
  int compare(const character_type* str, mem_size len) const noexcept {
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
  /* bool operator==(const BaseString& other) const noexcept {
        return memcmp(data_, other.data_, other.size_) == 0;
  }*/
  template <class TOther>
    requires(  //! std::same_as<TOther, BaseString> &&
        HasStringTraits<TOther, value_type>)
  bool operator==(const TOther& other) const {
    return memcmp(data_, other.c_str(), other.size()) == 0;
  }

  // access operator
  character_type& operator[](mem_size index) {
    BUGCHECK(index >= size_, "Index out of bounds");
    return data_[index];
  }
  const character_type& operator[](mem_size index) const {
    BUGCHECK(index >= size_, "Index out of bounds");
    return data_[index];
  }
  character_type* at(mem_size index) {
    BUGCHECK(index >= size_, "Index out of bounds");
    return data_ + index;
  }

  static constexpr mem_size npos = std::numeric_limits<mem_size>::max();
  BaseString substr(mem_size pos, mem_size count = npos) const {
    BUGCHECK(pos > size_, "Invalid position");

    if (count == npos) {
      count = size_ - pos;
    }

    BUGCHECK(pos + count > size_, "Invalid count");

    BaseString substr;
    substr.assign(data_ + pos, count);
    return substr;
  }

 private:
  // Allocates memory for the BaseString.
  void allocate(mem_size new_capacity_in_characters) {
    data_ = static_cast<character_type*>(TAllocator::Allocate(
        new_capacity_in_characters * sizeof(character_type)));
  }

  // Deallocates the memory used by the BaseString.
  void deallocate() {
    if (data_)
      TAllocator::Free(data_, (capacity_ * sizeof(character_type)) + 1);
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
  }

  // Reallocates the memory used by the BaseString to the given capacity.
  void reallocate(mem_size new_capacity_in_characters) {
    character_type* new_data =
        static_cast<character_type*>(TAllocator::Allocate(
            new_capacity_in_characters * sizeof(character_type)));
    memcpy(new_data, data_, size_ * sizeof(character_type));
    TAllocator::Free(data_, capacity_ * sizeof(character_type));
    data_ = new_data;
    capacity_ = new_capacity_in_characters - 1;  // hackfix;
  }

  character_type* data_ = nullptr;
  mem_size size_ = 0;
  mem_size capacity_ = 0;
};

}  // namespace base