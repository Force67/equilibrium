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

#define HAS_BASE_STRING_TRAITS 1

namespace base {
template <typename T, typename TEncoding>
concept HasStringTraits = requires(T& t) {
  t.data();
  t.c_str();
  t.size();
};

template <typename TInputIterator, typename T>
inline TInputIterator find(TInputIterator first, TInputIterator last, const T& value) {
  for (; first != last; ++first) {
    if (*first == value) {
      return first;
    }
  }
  return last;
}

template <typename TChar,
          typename TSizeType = mem_size,
          class TAllocator = base::DefaultAllocator>
class BasicBaseString {
 public:
  using character_type = TChar;
  using allocator_type = TAllocator;
  using value_type = character_type;
  using size_type = TSizeType;
  static constexpr size_type npos = base::MinMax<size_type>::max();

 private:
  // This structure defines the memory footprint of the string object.
  // On a 64-bit system, this is typically 24 bytes.
  struct LargeLayout {
    character_type* data_;
    size_type size_;
    size_type capacity_;
  };

  // The small string stores its data inside the object's footprint.
  // The capacity is the total size minus one byte for the size/flag field.
  static constexpr size_type kSmallCapacity =
      (sizeof(LargeLayout) - 1) / sizeof(character_type);

  // The last byte of a small string stores its size and the mode flag.
  // The MSB is the flag: 1 for Large, 0 for Small.
  // The remaining 7 bits store the size of the small string.
  static constexpr unsigned char kLargeFlag = 0x80;

  union {
    LargeLayout large_;
    struct {
      character_type data_[kSmallCapacity];
      unsigned char size_and_flag_;
    } small_;
  };

  // SSO (Small String Optimization) Helper Functions
  bool is_large() const noexcept { return (small_.size_and_flag_ & kLargeFlag) != 0; }

  size_type get_size() const noexcept {
    return is_large() ? large_.size_ : (small_.size_and_flag_ & ~kLargeFlag);
  }

  character_type* get_data() noexcept { return is_large() ? large_.data_ : small_.data_; }

  const character_type* get_data() const noexcept {
    return is_large() ? large_.data_ : small_.data_;
  }

  size_type get_capacity() const noexcept {
    return is_large() ? large_.capacity_ : kSmallCapacity;
  }

  void set_size(size_type new_size) {
    if (is_large()) {
      large_.size_ = new_size;
    } else {
      small_.size_and_flag_ = (unsigned char)(new_size & ~kLargeFlag);
    }
  }

  void ensure_null_terminated() noexcept { get_data()[get_size()] = '\0'; }

  void switch_to_large(size_type required_capacity) {
    character_type buffer_backup[kSmallCapacity];
    const size_type old_size = get_size();
    memcpy(buffer_backup, small_.data_, old_size * sizeof(character_type));

    // Geometric growth strategy
    size_type new_capacity = required_capacity + (required_capacity / 2);
    character_type* new_data = static_cast<character_type*>(
        TAllocator::Allocate((new_capacity + 1) * sizeof(character_type)));

    memcpy(new_data, buffer_backup, old_size * sizeof(character_type));

    large_.data_ = new_data;
    large_.size_ = old_size;
    large_.capacity_ = new_capacity;
    small_.size_and_flag_ |= kLargeFlag;  // Set flag to indicate large mode

    ensure_null_terminated();
  }

  void deallocate_large() {
    if (is_large()) {
      TAllocator::Free(large_.data_, (large_.capacity_ + 1) * sizeof(character_type));
    }
  }

 public:
  // -- Constructors and Destructor --

  BasicBaseString() noexcept {
    small_.size_and_flag_ = 0;  // is_small, size = 0
    ensure_null_terminated();
  }

  explicit BasicBaseString(const character_type* str) {
    small_.size_and_flag_ = 0;
    ensure_null_terminated();
    assign(str);
  }

  explicit BasicBaseString(const character_type* str, size_type len_in_characters) {
    small_.size_and_flag_ = 0;
    ensure_null_terminated();
    assign(str, len_in_characters);
  }

  explicit BasicBaseString(const character_type* begin, const character_type* end) {
    small_.size_and_flag_ = 0;
    ensure_null_terminated();
    assign(begin, end - begin);
  }

  template <size_type N>
  BasicBaseString(const character_type (&arr)[N]) {
    small_.size_and_flag_ = 0;
    ensure_null_terminated();
    assign(arr, N > 0 ? N - 1 : 0);
  }

  BasicBaseString(const BasicBaseString& other) {
    if (other.is_large()) {
      assign(other.large_.data_, other.large_.size_);
    } else {
      memcpy(this, &other, sizeof(other));
    }
  }

  template <class TOther>
    requires(base::HasStringTraits<TOther, value_type>)
  BasicBaseString(const TOther& other) {
    small_.size_and_flag_ = 0;
    ensure_null_terminated();
    assign(other.c_str(), other.size());
  }

  BasicBaseString(BasicBaseString&& other) noexcept {
    memcpy(this, &other, sizeof(*this));
    // Set the moved-from object to a valid empty state
    other.small_.size_and_flag_ = 0;
    other.ensure_null_terminated();
  }

  ~BasicBaseString() { deallocate_large(); }

  // -- Assignment Operators --

  BasicBaseString& operator=(const BasicBaseString& other) {
    if (this != &other) {
      assign(other.get_data(), other.get_size());
    }
    return *this;
  }

  BasicBaseString& operator=(const character_type* str) {
    assign(str);
    return *this;
  }

  BasicBaseString& operator=(BasicBaseString&& other) noexcept {
    if (this != &other) {
      deallocate_large();
      memcpy(this, &other, sizeof(*this));
      other.small_.size_and_flag_ = 0;
      other.ensure_null_terminated();
    }
    return *this;
  }

  void assign(const character_type* str, size_type len) {
    if (str == nullptr || len == 0) {
      clear();
      return;
    }
    if (len > get_capacity()) {
      deallocate_large();
      size_type new_capacity = len;
      large_.data_ = static_cast<character_type*>(
          TAllocator::Allocate((new_capacity + 1) * sizeof(character_type)));
      large_.capacity_ = new_capacity;
      small_.size_and_flag_ |= kLargeFlag;
    }
    memcpy(get_data(), str, len * sizeof(character_type));
    set_size(len);
    ensure_null_terminated();
  }
  void assign(const character_type* start, const character_type* end) {
    assign(start, end - start);
  }

  void assign(const character_type* str) { assign(str, base::CountStringLength(str)); }

  // -- Element Access and Iterators --

  const character_type* c_str() const noexcept { return get_data(); }
  character_type* data() noexcept { return get_data(); }
  const character_type* data() const noexcept { return get_data(); }

  const character_type* begin() const noexcept { return get_data(); }
  const character_type* end() const noexcept { return get_data() + get_size(); }
  character_type* begin() noexcept { return get_data(); }
  character_type* end() noexcept { return get_data() + get_size(); }

  character_type& back() {
    BASE_BUGCHECK(!empty(), "Cannot access .back() of an empty string");
    return get_data()[get_size() - 1];
  }

  const character_type& back() const {
    BASE_BUGCHECK(!empty(), "Cannot access .back() of an empty string");
    return get_data()[get_size() - 1];
  }

  character_type& operator[](size_type index) {
    BASE_BUGCHECK(index < get_size(), "Index out of bounds");
    return get_data()[index];
  }
  const character_type& operator[](size_type index) const {
    BASE_BUGCHECK(index < get_size(), "Index out of bounds");
    return get_data()[index];
  }

  character_type& at(size_type pos) {
    BASE_BUGCHECK(pos < get_size(), "Position out of bounds");
    return get_data()[pos];
  }

  const character_type& at(size_type pos) const {
    BASE_BUGCHECK(pos < get_size(), "Position out of bounds");
    return get_data()[pos];
  }

  // -- Capacity and Size --

  size_type size() const noexcept { return get_size(); }
  size_type length() const noexcept { return get_size(); }
  size_type byte_size() const noexcept { return get_size() * sizeof(character_type); }
  bool empty() const noexcept { return get_size() == 0; }
  size_type capacity() const noexcept { return get_capacity(); }

  void reserve(size_type new_capacity) {
    if (new_capacity > get_capacity()) {
      if (!is_large()) {
        switch_to_large(new_capacity);
      } else {
        const size_type old_size = get_size();
        character_type* new_data = static_cast<character_type*>(
            TAllocator::Allocate((new_capacity + 1) * sizeof(character_type)));
        memcpy(new_data, large_.data_, old_size * sizeof(character_type));
        deallocate_large();
        large_.data_ = new_data;
        large_.size_ = old_size;
        large_.capacity_ = new_capacity;
        small_.size_and_flag_ |= kLargeFlag;  // ensure flag is set
        ensure_null_terminated();
      }
    }
  }

  void resize(size_type new_size) {
    const size_type old_size = get_size();
    if (new_size > old_size) {
      reserve(new_size);
      memset(get_data() + old_size, 0, (new_size - old_size) * sizeof(character_type));
    }
    set_size(new_size);
    ensure_null_terminated();
  }

  void clear() {
    deallocate_large();
    small_.size_and_flag_ = 0;  // to empty small string
    ensure_null_terminated();
  }

  void shrink_to_fit() {
    if (!is_large() || get_size() == get_capacity()) {
      return;
    }
    const size_type current_size = get_size();
    if (current_size <= kSmallCapacity) {
      // Transition from large to small
      character_type* old_data = large_.data_;
      memcpy(small_.data_, old_data, current_size * sizeof(character_type));
      small_.size_and_flag_ = (unsigned char)current_size;  // Now small
      ensure_null_terminated();
      TAllocator::Free(old_data, (large_.capacity_ + 1) * sizeof(character_type));
    } else {
      // Shrink the large buffer
      character_type* new_data = static_cast<character_type*>(
          TAllocator::Allocate((current_size + 1) * sizeof(character_type)));
      memcpy(new_data, large_.data_, current_size * sizeof(character_type));
      deallocate_large();
      large_.data_ = new_data;
      large_.size_ = current_size;
      large_.capacity_ = current_size;
      small_.size_and_flag_ |= kLargeFlag;
      ensure_null_terminated();
    }
  }

  // -- Modification Operations --

  BasicBaseString& operator+=(const BasicBaseString& other) {
    append(other.get_data(), other.get_size());
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

  void append(const character_type* str, size_type count) {
    if (count == 0)
      return;
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    reserve(new_size);
    memcpy(get_data() + old_size, str, count * sizeof(character_type));
    set_size(new_size);
    ensure_null_terminated();
  }
  void append(const character_type* str) { append(str, base::CountStringLength(str)); }

  void push_back(character_type c) {
    const size_type old_size = get_size();
    if (old_size == get_capacity()) {
      reserve(old_size + 1);
    }
    get_data()[old_size] = c;
    set_size(old_size + 1);
    ensure_null_terminated();
  }

  void insert(size_type pos, size_type count, character_type c) {
    BASE_BUGCHECK(pos <= get_size(), "Invalid position");
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    reserve(new_size);
    character_type* d = get_data();
    memmove(d + pos + count, d + pos, (old_size - pos) * sizeof(character_type));
    memset(d + pos, c, count * sizeof(character_type));
    set_size(new_size);
    ensure_null_terminated();
  }

  void erase(size_type pos = 0, size_type count = npos) {
    const size_type current_size = get_size();
    BASE_BUGCHECK(pos <= current_size, "Invalid position");
    count = std::min(count, current_size - pos);
    if (count == 0)
      return;

    character_type* d = get_data();
    memmove(d + pos, d + pos + count,
            (current_size - pos - count) * sizeof(character_type));
    set_size(current_size - count);
    ensure_null_terminated();
  }
  void erase(const character_type* p) {
    const size_type pos = p - get_data();
    BASE_BUGCHECK(pos < get_size(), "Pointer out of bounds");
    erase(pos, 1);
  }
  void erase(const character_type* start, const character_type* end) {
    BASE_BUGCHECK(start < end, "Invalid range");
    const size_type pos = start - get_data();
    BASE_BUGCHECK(pos < get_size(), "Pointer out of bounds");
    erase(pos, end - start);
  }

  void remove_suffix(size_type n) {
    const size_type current_size = get_size();
    BASE_BUGCHECK(n <= current_size, "Invalid count");
    set_size(current_size - n);
    ensure_null_terminated();
  }

  BasicBaseString substr(size_type pos = 0, size_type count = npos) const {
    const size_type current_size = get_size();
    BASE_BUGCHECK(pos <= current_size, "Invalid position");
    count = std::min(count, current_size - pos);
    return BasicBaseString(get_data() + pos, count);
  }

  // -- Search and Compare --

  int compare(const BasicBaseString& other) const noexcept {
    const size_type left_size = get_size();
    const size_type right_size = other.get_size();
    const size_type min_size = std::min(left_size, right_size);
    int result = memcmp(get_data(), other.get_data(), min_size * sizeof(character_type));
    if (result != 0)
      return result;
    if (left_size < right_size)
      return -1;
    if (left_size > right_size)
      return 1;
    return 0;
  }

  int compare(const character_type* str) const noexcept {
    const size_type left_size = get_size();
    const size_type right_size = base::CountStringLength(str);
    const size_type min_size = std::min(left_size, right_size);
    int result = memcmp(get_data(), str, min_size * sizeof(character_type));
    if (result != 0)
      return result;
    if (left_size < right_size)
      return -1;
    if (left_size > right_size)
      return 1;
    return 0;
  }

  int compare(const character_type* str, size_type count) const noexcept {
    const size_type left_size = get_size();
    const size_type min_size = std::min(left_size, count);
    int result = memcmp(get_data(), str, min_size * sizeof(character_type));
    if (result != 0)
      return result;
    if (left_size < count)
      return -1;
    if (left_size > count)
      return 1;
    return 0;
  }

  // for backwards compat
  int compare(size_type offset,
              size_type count,
              const character_type* str) const noexcept {
    BASE_BUGCHECK(offset < get_size(), "Offset out of bounds");
    const size_type left_size = get_size() - offset;
    const size_type min_size = std::min(left_size, count);
    int result = memcmp(get_data() + offset, str, min_size * sizeof(character_type));
    if (result != 0)
      return result;
    if (left_size < count)
      return -1;
    if (left_size > count)
      return 1;
    return 0;
  }

  int compare(size_type pos, size_type len, const BasicBaseString& str) const noexcept {
    BASE_BUGCHECK(pos <= get_size(), "Position out of bounds");

    const size_type rlen = std::min(len, get_size() - pos);
    const size_type other_len = str.get_size();
    const size_type min_len = std::min(rlen, other_len);

    int result =
        memcmp(get_data() + pos, str.get_data(), min_len * sizeof(character_type));

    if (result != 0) {
      return result;
    }

    if (rlen < other_len) {
      return -1;
    }
    if (rlen > other_len) {
      return 1;
    }

    return 0;
  }
  size_type find(character_type c, size_type pos = 0) const {
    if (pos >= get_size())
      return npos;
    const character_type* result =
        base::find(get_data() + pos, get_data() + get_size(), c);
    return result == end() ? npos : result - begin();
  }

  size_type find_last_of(character_type c, size_type pos = npos) const {
    const size_type current_size = get_size();
    if (current_size == 0)
      return npos;
    size_type search_end = (pos == npos || pos >= current_size) ? current_size - 1 : pos;
    for (size_type i = search_end; i != static_cast<size_type>(-1); --i) {
      if (get_data()[i] == c) {
        return i;
      }
    }
    return npos;
  }

  // kSeparators, BufferType::npos, kSeparatorsLength - 1
  size_type find_last_of(const character_type* s,
                         size_type pos = npos,
                         size_type s_len = npos) const {
    const size_type current_size = get_size();
    if (current_size == 0 || s == nullptr || s_len == 0)
      return npos;
    if (pos == npos || pos >= current_size)
      pos = current_size - 1;

    for (size_type i = pos; i != static_cast<size_type>(-1); --i) {
      if (base::find(s, s + s_len, get_data()[i]) != s + s_len) {
        return i;
      }
    }
    return npos;
  }
};

// -- Non-Member Comparison Operators --
template <typename CharT, typename TSizeType, class TAllocator>
bool operator==(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  if (lhs.size() != rhs.size())
    return false;
  return memcmp(lhs.data(), rhs.data(), lhs.byte_size()) == 0;
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator==(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const CharT* rhs) {
  if (lhs.empty() && rhs == nullptr)
    return true;
  if (rhs == nullptr || base::CountStringLength(rhs) != lhs.size())
    return false;
  return memcmp(lhs.data(), rhs, lhs.byte_size()) == 0;
}

template <typename CharT, typename TSizeType, class TAllocator, typename TOtherString>
  requires(HasStringTraits<TOtherString, CharT>)
bool operator==(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const TOtherString& rhs) {
  const auto lhs_size = lhs.size();
  const auto rhs_size = rhs.size();

  if (lhs_size != rhs_size) {
    return false;
  }

  // If both strings are empty, they are equal.
  if (lhs_size == 0) {
    return true;
  }

  return memcmp(lhs.data(), rhs.data(), lhs_size * sizeof(CharT)) == 0;
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator!=(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  return !(lhs == rhs);
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator<(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
               const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  return lhs.compare(rhs) < 0;
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator<=(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  return lhs.compare(rhs) <= 0;
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator>(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
               const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  return lhs.compare(rhs) > 0;
}

template <typename CharT, typename TSizeType, class TAllocator>
bool operator>=(const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
                const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  return lhs.compare(rhs) >= 0;
}

// -- Non-Member Concatenation --

template <typename CharT, typename TSizeType, class TAllocator>
BasicBaseString<CharT, TSizeType, TAllocator> operator+(
    const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
    const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  BasicBaseString<CharT, TSizeType, TAllocator> result;
  result.reserve(lhs.size() + rhs.size());
  result.append(lhs.data(), lhs.size());
  result.append(rhs.data(), rhs.size());
  return result;
}

template <typename CharT, typename TSizeType, class TAllocator>
BasicBaseString<CharT, TSizeType, TAllocator> operator+(
    const BasicBaseString<CharT, TSizeType, TAllocator>& lhs,
    const CharT* rhs) {
  BasicBaseString<CharT, TSizeType, TAllocator> result(lhs);
  result.append(rhs);
  return result;
}

template <typename CharT, typename TSizeType, class TAllocator>
BasicBaseString<CharT, TSizeType, TAllocator> operator+(
    const CharT* lhs,
    const BasicBaseString<CharT, TSizeType, TAllocator>& rhs) {
  BasicBaseString<CharT, TSizeType, TAllocator> result(lhs);
  result.append(rhs);
  return result;
}

}  // namespace base