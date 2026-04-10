// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// BasicSmallString<TChar, N>
//
// Like BasicBaseString, but the inline (small) buffer is templated to hold
// up to N characters. When the string outgrows N it spills onto the heap
// via TAllocator. Use it for things like file paths or names where the
// common case is comfortably larger than the 22-byte SSO buffer of
// BasicBaseString but you still want to avoid heap traffic.
//
// Layout:
//   - Inline mode: SmallRep { character_type data[N+1]; size_type size; }
//   - Heap  mode: LargeRep  { character_type* data;     size_type size, capacity; }
//   - Discriminator: explicit bool is_large_ at the tail.
//
// We deliberately avoid the bit-stuffing trick used by BasicBaseString — for
// large N the bookkeeping wins us nothing and bool is_large_ is unambiguous.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/numeric_limits.h>
#include <base/containers/container_traits.h>
#include <base/math/value_bounds.h>
#include <base/strings/base_string.h>  // for HasStringTraits, base::find
#include <base/strings/char_algorithms.h>

#include <cstring>

namespace base {

template <typename TChar,
          mem_size N,
          class TAllocator = base::DefaultAllocator>
class BasicSmallString {
  static_assert(N > 0, "BasicSmallString requires N > 0");

 public:
  using character_type = TChar;
  using value_type = character_type;
  // Deliberately u32: SmallString isn't intended to back >4GB blobs, and
  // shrinking the size/capacity fields trims 8 bytes from the inline layout.
  using size_type = u32;
  using allocator_type = TAllocator;
  static constexpr size_type npos = base::MinMax<size_type>::max();
  static constexpr size_type kInlineCapacity = static_cast<size_type>(N);
  static_assert(N <= base::MinMax<size_type>::max() - 1,
                "BasicSmallString N must fit in size_type");

 private:
  struct LargeRep {
    character_type* data_;
    size_type size_;
    size_type capacity_;
  };

  struct SmallRep {
    character_type data_[N + 1];  // chars + null terminator
    size_type size_;
  };

  union {
    LargeRep large_;
    SmallRep small_;
  };
  bool is_large_;

  // -- Internal helpers --

  character_type* mutable_data() noexcept {
    return is_large_ ? large_.data_ : small_.data_;
  }
  const character_type* const_data() const noexcept {
    return is_large_ ? large_.data_ : small_.data_;
  }
  size_type get_size() const noexcept {
    return is_large_ ? large_.size_ : small_.size_;
  }
  size_type get_capacity() const noexcept {
    return is_large_ ? large_.capacity_ : kInlineCapacity;
  }
  void set_size(size_type new_size) noexcept {
    if (is_large_) {
      large_.size_ = new_size;
    } else {
      small_.size_ = new_size;
    }
  }
  void terminate() noexcept { mutable_data()[get_size()] = character_type{}; }

  void init_empty() noexcept {
    is_large_ = false;
    small_.size_ = 0;
    small_.data_[0] = character_type{};
  }

  void deallocate_large_if_needed() {
    if (is_large_) {
      TAllocator::Free(large_.data_,
                       (large_.capacity_ + 1) * sizeof(character_type));
    }
  }

  // Grow capacity to at least `required` characters. Preserves contents.
  void grow_to(size_type required) {
    if (required <= get_capacity()) return;

    // Geometric growth: 1.5x rounded up.
    size_type new_capacity = required + (required / 2);
    character_type* new_data = static_cast<character_type*>(
        TAllocator::Allocate((new_capacity + 1) * sizeof(character_type)));

    const size_type cur_size = get_size();
    if (cur_size > 0) {
      memcpy(new_data, const_data(), cur_size * sizeof(character_type));
    }
    new_data[cur_size] = character_type{};

    deallocate_large_if_needed();
    large_.data_ = new_data;
    large_.size_ = cur_size;
    large_.capacity_ = new_capacity;
    is_large_ = true;
  }

 public:
  // -- Constructors / destructor --

  BasicSmallString() noexcept { init_empty(); }

  BasicSmallString(const character_type* str) {
    init_empty();
    if (str) assign(str);
  }

  BasicSmallString(const character_type* str, size_type len) {
    init_empty();
    assign(str, len);
  }

  BasicSmallString(const character_type* begin, const character_type* end) {
    init_empty();
    assign(begin, static_cast<size_type>(end - begin));
  }

  BasicSmallString(size_type count, character_type c) {
    init_empty();
    if (count > 0) {
      grow_to(count);
      character_type* d = mutable_data();
      for (size_type i = 0; i < count; ++i) d[i] = c;
      set_size(count);
      terminate();
    }
  }

  template <mem_size ArraySize>
  BasicSmallString(const character_type (&arr)[ArraySize]) {
    init_empty();
    assign(arr, static_cast<size_type>(base::CountStringLength(
                    arr, ArraySize > 0 ? ArraySize - 1 : 0)));
  }

  BasicSmallString(const BasicSmallString& other) {
    init_empty();
    assign(other.const_data(), other.get_size());
  }

  template <class TOther>
    requires(base::HasStringTraits<TOther, value_type>)
  BasicSmallString(const TOther& other) {
    init_empty();
    assign(other.c_str(), static_cast<size_type>(other.size()));
  }

  BasicSmallString(BasicSmallString&& other) noexcept {
    if (other.is_large_) {
      // Steal the heap buffer.
      large_ = other.large_;
      is_large_ = true;
      other.init_empty();
    } else {
      is_large_ = false;
      small_.size_ = other.small_.size_;
      memcpy(small_.data_, other.small_.data_,
             (other.small_.size_ + 1) * sizeof(character_type));
      other.init_empty();
    }
  }

  ~BasicSmallString() { deallocate_large_if_needed(); }

  // -- Assignment --

  BasicSmallString& operator=(const BasicSmallString& other) {
    if (this != &other) assign(other.const_data(), other.get_size());
    return *this;
  }

  BasicSmallString& operator=(const character_type* str) {
    assign(str);
    return *this;
  }

  BasicSmallString& operator=(BasicSmallString&& other) noexcept {
    if (this != &other) {
      deallocate_large_if_needed();
      if (other.is_large_) {
        large_ = other.large_;
        is_large_ = true;
      } else {
        is_large_ = false;
        small_.size_ = other.small_.size_;
        memcpy(small_.data_, other.small_.data_,
               (other.small_.size_ + 1) * sizeof(character_type));
      }
      other.init_empty();
    }
    return *this;
  }

  void assign(const character_type* str, size_type len) {
    if (str == nullptr || len == 0) {
      clear();
      return;
    }
    if (len > get_capacity()) grow_to(len);
    memcpy(mutable_data(), str, len * sizeof(character_type));
    set_size(len);
    terminate();
  }
  void assign(const character_type* start, const character_type* end) {
    assign(start, static_cast<size_type>(end - start));
  }
  void assign(const character_type* str) {
    assign(str, static_cast<size_type>(base::CountStringLength(str)));
  }

  // -- Access --

  const character_type* c_str() const noexcept { return const_data(); }
  character_type* data() noexcept { return mutable_data(); }
  const character_type* data() const noexcept { return const_data(); }

  character_type* begin() noexcept { return mutable_data(); }
  character_type* end() noexcept { return mutable_data() + get_size(); }
  const character_type* begin() const noexcept { return const_data(); }
  const character_type* end() const noexcept {
    return const_data() + get_size();
  }

  character_type& operator[](size_type i) {
    BASE_BUGCHECK(i < get_size(), "Index out of bounds");
    return mutable_data()[i];
  }
  const character_type& operator[](size_type i) const {
    BASE_BUGCHECK(i < get_size(), "Index out of bounds");
    return const_data()[i];
  }
  character_type& at(size_type i) { return (*this)[i]; }
  const character_type& at(size_type i) const { return (*this)[i]; }

  character_type& back() {
    BASE_BUGCHECK(!empty(), "back() on empty string");
    return mutable_data()[get_size() - 1];
  }
  const character_type& back() const {
    BASE_BUGCHECK(!empty(), "back() on empty string");
    return const_data()[get_size() - 1];
  }

  // -- Capacity --

  size_type size() const noexcept { return get_size(); }
  size_type length() const noexcept { return get_size(); }
  size_type byte_size() const noexcept {
    return get_size() * sizeof(character_type);
  }
  bool empty() const noexcept { return get_size() == 0; }
  size_type capacity() const noexcept { return get_capacity(); }
  static constexpr size_type inline_capacity() noexcept { return N; }
  bool is_inline() const noexcept { return !is_large_; }

  void reserve(size_type new_capacity) {
    if (new_capacity > get_capacity()) grow_to(new_capacity);
  }

  void resize(size_type new_size) {
    const size_type old_size = get_size();
    if (new_size > old_size) {
      reserve(new_size);
      memset(mutable_data() + old_size, 0,
             (new_size - old_size) * sizeof(character_type));
    }
    set_size(new_size);
    terminate();
  }

  void clear() noexcept {
    if (is_large_) {
      large_.size_ = 0;
      large_.data_[0] = character_type{};
    } else {
      small_.size_ = 0;
      small_.data_[0] = character_type{};
    }
  }

  void shrink_to_fit() {
    if (!is_large_) return;
    const size_type cur = large_.size_;
    if (cur <= kInlineCapacity) {
      // Move heap contents back inline.
      character_type* old_data = large_.data_;
      const size_type old_capacity = large_.capacity_;
      is_large_ = false;
      small_.size_ = cur;
      memcpy(small_.data_, old_data, cur * sizeof(character_type));
      small_.data_[cur] = character_type{};
      TAllocator::Free(old_data, (old_capacity + 1) * sizeof(character_type));
    } else if (cur < large_.capacity_) {
      character_type* new_data = static_cast<character_type*>(
          TAllocator::Allocate((cur + 1) * sizeof(character_type)));
      memcpy(new_data, large_.data_, cur * sizeof(character_type));
      new_data[cur] = character_type{};
      TAllocator::Free(large_.data_,
                       (large_.capacity_ + 1) * sizeof(character_type));
      large_.data_ = new_data;
      large_.capacity_ = cur;
    }
  }

  // -- Modification --

  BasicSmallString& operator+=(const BasicSmallString& other) {
    append(other.const_data(), other.get_size());
    return *this;
  }
  BasicSmallString& operator+=(const character_type* str) {
    append(str);
    return *this;
  }
  BasicSmallString& operator+=(character_type c) {
    push_back(c);
    return *this;
  }

  void append(const character_type* str, size_type count) {
    if (count == 0) return;
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    if (new_size > get_capacity()) grow_to(new_size);
    memcpy(mutable_data() + old_size, str, count * sizeof(character_type));
    set_size(new_size);
    terminate();
  }
  void append(const character_type* str) {
    append(str, static_cast<size_type>(base::CountStringLength(str)));
  }

  void push_back(character_type c) {
    const size_type old_size = get_size();
    if (old_size + 1 > get_capacity()) grow_to(old_size + 1);
    mutable_data()[old_size] = c;
    set_size(old_size + 1);
    terminate();
  }

  void insert(size_type pos, size_type count, character_type c) {
    BASE_BUGCHECK(pos <= get_size(), "Invalid insert position");
    if (count == 0) return;
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    if (new_size > get_capacity()) grow_to(new_size);
    character_type* d = mutable_data();
    memmove(d + pos + count, d + pos, (old_size - pos) * sizeof(character_type));
    for (size_type i = 0; i < count; ++i) d[pos + i] = c;
    set_size(new_size);
    terminate();
  }

  void erase(size_type pos = 0, size_type count = npos) {
    const size_type cur = get_size();
    BASE_BUGCHECK(pos <= cur, "Invalid erase position");
    count = base::Min(count, cur - pos);
    if (count == 0) return;
    character_type* d = mutable_data();
    memmove(d + pos, d + pos + count,
            (cur - pos - count) * sizeof(character_type));
    set_size(cur - count);
    terminate();
  }

  void remove_suffix(size_type n) {
    const size_type cur = get_size();
    BASE_BUGCHECK(n <= cur, "remove_suffix out of range");
    set_size(cur - n);
    terminate();
  }

  BasicSmallString substr(size_type pos = 0, size_type count = npos) const {
    const size_type cur = get_size();
    BASE_BUGCHECK(pos <= cur, "substr out of range");
    count = base::Min(count, cur - pos);
    return BasicSmallString(const_data() + pos, count);
  }

  // -- Compare --

  int compare(const BasicSmallString& other) const noexcept {
    return compare(other.const_data(), other.get_size());
  }
  int compare(const character_type* str) const noexcept {
    return compare(str, static_cast<size_type>(base::CountStringLength(str)));
  }
  int compare(const character_type* str, size_type count) const noexcept {
    const size_type left_size = get_size();
    const size_type min_size = base::Min(left_size, count);
    int result = memcmp(const_data(), str, min_size * sizeof(character_type));
    if (result != 0) return result;
    if (left_size < count) return -1;
    if (left_size > count) return 1;
    return 0;
  }

  // -- Search --

  size_type find(character_type c, size_type pos = 0) const {
    const size_type cur = get_size();
    if (pos >= cur) return npos;
    const character_type* d = const_data();
    for (size_type i = pos; i < cur; ++i) {
      if (d[i] == c) return i;
    }
    return npos;
  }

  size_type find(const character_type* s, size_type pos = 0) const {
    if (!s) return npos;
    const size_type s_len = static_cast<size_type>(base::CountStringLength(s));
    const size_type cur = get_size();
    if (s_len == 0) return pos <= cur ? pos : npos;
    if (pos + s_len > cur) return npos;
    const character_type* d = const_data();
    for (size_type i = pos; i <= cur - s_len; ++i) {
      if (memcmp(d + i, s, s_len * sizeof(character_type)) == 0) return i;
    }
    return npos;
  }

  size_type find(const BasicSmallString& s, size_type pos = 0) const {
    return find(s.c_str(), pos);
  }

  size_type find_last_of(character_type c, size_type pos = npos) const {
    const size_type cur = get_size();
    if (cur == 0) return npos;
    size_type i = (pos == npos || pos >= cur) ? cur - 1 : pos;
    const character_type* d = const_data();
    for (;; --i) {
      if (d[i] == c) return i;
      if (i == 0) return npos;
    }
  }
};

// -- Non-member comparison --

template <typename TChar, mem_size N, class TAllocator>
bool operator==(const BasicSmallString<TChar, N, TAllocator>& lhs,
                const BasicSmallString<TChar, N, TAllocator>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  return memcmp(lhs.data(), rhs.data(), lhs.byte_size()) == 0;
}

template <typename TChar, mem_size N, class TAllocator>
bool operator==(const BasicSmallString<TChar, N, TAllocator>& lhs,
                const TChar* rhs) {
  if (rhs == nullptr) return lhs.empty();
  const auto rhs_len = base::CountStringLength(rhs);
  if (rhs_len != lhs.size()) return false;
  return memcmp(lhs.data(), rhs, lhs.byte_size()) == 0;
}

template <typename TChar, mem_size N, class TAllocator>
bool operator!=(const BasicSmallString<TChar, N, TAllocator>& lhs,
                const BasicSmallString<TChar, N, TAllocator>& rhs) {
  return !(lhs == rhs);
}

template <typename TChar, mem_size N, class TAllocator>
bool operator<(const BasicSmallString<TChar, N, TAllocator>& lhs,
               const BasicSmallString<TChar, N, TAllocator>& rhs) {
  return lhs.compare(rhs) < 0;
}
template <typename TChar, mem_size N, class TAllocator>
bool operator<=(const BasicSmallString<TChar, N, TAllocator>& lhs,
                const BasicSmallString<TChar, N, TAllocator>& rhs) {
  return lhs.compare(rhs) <= 0;
}
template <typename TChar, mem_size N, class TAllocator>
bool operator>(const BasicSmallString<TChar, N, TAllocator>& lhs,
               const BasicSmallString<TChar, N, TAllocator>& rhs) {
  return lhs.compare(rhs) > 0;
}
template <typename TChar, mem_size N, class TAllocator>
bool operator>=(const BasicSmallString<TChar, N, TAllocator>& lhs,
                const BasicSmallString<TChar, N, TAllocator>& rhs) {
  return lhs.compare(rhs) >= 0;
}

}  // namespace base
