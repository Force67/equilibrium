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
#include <base/check.h>
#include <base/numeric_limits.h>
#include <base/containers/container_traits.h>
#include <base/meta/traits.h>
#include <base/strings/char_algorithms.h>

#include <cstring>
#include <base/math/value_bounds.h>

#define HAS_BASE_STRING_TRAITS 1

namespace base {
template <typename T, typename TEncoding>
concept HasStringTraits = requires(T& t) {
  t.data();
  t.c_str();
  t.size();
};

// A non-owning character range: pointer + length, no allocator, no c_str()
// guarantee. std::string_view and base::StringRef both satisfy it; owning
// strings and base::Vector do not (they carry an allocator_type).
template <typename T, typename TEncoding>
concept StringViewLike = requires(const T& t) {
  { t.data() } -> base::SameAs<const TEncoding*>;
  t.size();
} && !requires { typename T::allocator_type; };

// The mirror of StringViewLike, for the implicit conversion out of a string:
// any view that can be built from (pointer, length) and owns nothing. Trivial
// copyability is what keeps owning strings out (they all have a user-provided
// copy constructor), so a base::String never silently converts into an
// allocating type.
template <typename T, typename TEncoding>
concept ConstructibleView =
    __is_constructible(T, const TEncoding*, mem_size) && __is_trivially_copyable(T);

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
  // Heap-mode footprint. On 64-bit with default size_type this is 24 bytes.
  struct LargeLayout {
    character_type* data_;
    size_type size_;
    size_type capacity_;
  };

  // The flag bit lives in the very last byte of the union, which (on
  // little-endian) overlaps the high byte of large_.capacity_. The MSB of
  // that byte is the is-large discriminator; the low 7 bits store the
  // small-mode size. So kSmallCapacity must fit in 7 bits.
  static constexpr unsigned char kLargeFlag = 0x80;

  // Inline char count (not counting the trailing null). Picked so that
  //   (kSmallCapacity + 1) chars + at least one tail byte (the flag)
  //   fit inside sizeof(LargeLayout).
  static constexpr size_type kSmallCapacity =
      (sizeof(LargeLayout) - 1) / sizeof(character_type) - 1;

  // Bytes after the inline char array. For `char` this is exactly 1
  // (the flag byte itself). For wider char types we need extra padding
  // bytes so the flag still lands at offset (sizeof(LargeLayout) - 1) —
  // i.e. always the very last byte of the union, always overlapping the
  // high byte of large_.capacity_ on little-endian.
  static constexpr size_type kSmallTailBytes =
      sizeof(LargeLayout) - (kSmallCapacity + 1) * sizeof(character_type);

  static_assert(kSmallTailBytes >= 1, "no room for the flag byte");
  static_assert(kSmallCapacity < 128, "kSmallCapacity must fit in 7 bits");

  union {
    LargeLayout large_;
    struct {
      // kSmallCapacity user chars + 1 null terminator slot.
      character_type data_[kSmallCapacity + 1];
      // Trailing padding bytes; the LAST byte is the flag/size byte.
      // For char this array is exactly [1]; for wide chars it's larger
      // and only the last element carries semantic information.
      unsigned char tail_[kSmallTailBytes];
    } small_;
  };

  // -- Layout helpers --

  unsigned char& flag_byte() noexcept {
    return small_.tail_[kSmallTailBytes - 1];
  }
  unsigned char flag_byte() const noexcept {
    return small_.tail_[kSmallTailBytes - 1];
  }

  bool is_large() const noexcept { return (flag_byte() & kLargeFlag) != 0; }

  size_type get_size() const noexcept {
    return is_large() ? large_.size_
                      : static_cast<size_type>(flag_byte() & ~kLargeFlag);
  }

  character_type* get_data() noexcept {
    return is_large() ? large_.data_ : small_.data_;
  }
  const character_type* get_data() const noexcept {
    return is_large() ? large_.data_ : small_.data_;
  }

  size_type get_capacity() const noexcept {
    if (!is_large()) return kSmallCapacity;
    // Mask out the flag bit (bit 63 of capacity_, i.e. bit 7 of its top byte
    // on little-endian).
    return large_.capacity_ &
           ~(static_cast<size_type>(kLargeFlag)
             << ((sizeof(size_type) - 1) * 8));
  }

  void set_size(size_type new_size) noexcept {
    if (is_large()) {
      large_.size_ = new_size;
    } else {
      flag_byte() = static_cast<unsigned char>(new_size & ~kLargeFlag);
    }
  }

  void ensure_null_terminated() noexcept {
    get_data()[get_size()] = character_type{};
  }

  void init_empty() noexcept {
    flag_byte() = 0;
    small_.data_[0] = character_type{};
  }

  // -- Capacity / allocation helpers --

  static size_type grow_capacity(size_type required) noexcept {
    // Geometric growth (1.5x) with an overflow guard.
    const size_type kMax = base::MinMax<size_type>::max();
    if (required > kMax - required / 2) return required;
    return required + required / 2;
  }

  // Allocates target_capacity + 1 chars, copies our current contents,
  // frees the old buffer (if any), and switches us into large mode.
  // The old buffer is freed AFTER the copy, so callers can pass aliased
  // sources separately (see assign/append).
  void realloc_to(size_type target_capacity) {
    BASE_BUGCHECK(target_capacity >= get_size(),
                  "realloc_to would lose data");
    character_type* new_data = static_cast<character_type*>(
        TAllocator::Allocate((target_capacity + 1) * sizeof(character_type)));
    const size_type cur_size = get_size();
    if (cur_size > 0) {
      memcpy(new_data, get_data(), cur_size * sizeof(character_type));
    }
    new_data[cur_size] = character_type{};

    deallocate_large();
    large_.data_ = new_data;
    large_.size_ = cur_size;
    large_.capacity_ = target_capacity;
    // OR-set the flag so we don't clobber the high bits of capacity_.
    flag_byte() |= kLargeFlag;
  }

  // Geometric grow-to-fit; used by all auto-growing modifiers.
  void grow_to_at_least(size_type min_capacity) {
    if (min_capacity > get_capacity()) {
      realloc_to(grow_capacity(min_capacity));
    }
  }

  void deallocate_large() {
    if (is_large()) {
      TAllocator::Free(large_.data_,
                       (get_capacity() + 1) * sizeof(character_type));
    }
  }

 public:
  // -- Constructors and Destructor --

  BasicBaseString() noexcept { init_empty(); }

  // Implicit from const char* for std::string-like ergonomics
  BasicBaseString(const character_type* str) {
    init_empty();
    if (str) assign(str);
  }

  BasicBaseString(const character_type* str, size_type len_in_characters) {
    init_empty();
    assign(str, len_in_characters);
  }

  BasicBaseString(const character_type* begin, const character_type* end) {
    init_empty();
    assign(begin, static_cast<size_type>(end - begin));
  }

  // Fill constructor: creates a string of `count` copies of `c`.
  BasicBaseString(size_type count, character_type c) {
    init_empty();
    if (count > 0) {
      grow_to_at_least(count);
      character_type* d = get_data();
      // Scalar loop — memset would only write the low byte of c, which is
      // wrong for wchar_t/char16_t/char32_t.
      for (size_type i = 0; i < count; ++i) d[i] = c;
      set_size(count);
      ensure_null_terminated();
    }
  }

  template <size_type N>
  BasicBaseString(const character_type (&arr)[N]) {
    init_empty();
    // Use actual string length, not array size. A char buf[64] = "hello"
    // has N=64 but the string is only 5 chars.
    assign(arr, base::CountStringLength(arr, N > 0 ? N - 1 : 0));
  }

  BasicBaseString(const BasicBaseString& other) {
    init_empty();
    if (other.is_large()) {
      assign(other.large_.data_, other.large_.size_);
    } else {
      // Trivial bytewise copy is sound: union members are trivial.
      memcpy(this, &other, sizeof(other));
    }
  }

  template <class TOther>
    requires(base::HasStringTraits<TOther, value_type>)
  BasicBaseString(const TOther& other) {
    init_empty();
    assign(other.c_str(), static_cast<size_type>(other.size()));
  }

  // From any non-owning character range (std::string_view, base::StringRef).
  // Views carry no null terminator, so this copies by (data, size).
  template <class TView>
    requires(base::StringViewLike<TView, character_type> &&
             !base::HasStringTraits<TView, character_type>)
  BasicBaseString(const TView& view) {
    init_empty();
    assign(view.data(), static_cast<size_type>(view.size()));
  }

  // Implicit conversion to any view over our characters, without naming (or
  // including) the view type: it only has to be constructible from
  // (pointer, length) and own nothing. This is what lets a base::String be
  // passed straight to an API taking std::string_view.
  template <class TView>
    requires(base::ConstructibleView<TView, character_type> &&
             !base::is_same_v<TView, BasicBaseString>)
  constexpr operator TView() const {
    return TView(get_data(), get_size());
  }

  BasicBaseString(BasicBaseString&& other) noexcept {
    memcpy(this, &other, sizeof(*this));
    other.init_empty();
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
      other.init_empty();
    }
    return *this;
  }

  void assign(const character_type* str, size_type len) {
    if (str == nullptr || len == 0) {
      clear();
      return;
    }
    if (len > get_capacity()) {
      // Source-aliasing safe: allocate the new buffer first, copy from src
      // (which may point inside our old buffer — still valid), and only then
      // free the old buffer.
      character_type* new_data = static_cast<character_type*>(
          TAllocator::Allocate((len + 1) * sizeof(character_type)));
      memcpy(new_data, str, len * sizeof(character_type));
      new_data[len] = character_type{};
      deallocate_large();
      large_.data_ = new_data;
      large_.size_ = len;
      large_.capacity_ = len;
      flag_byte() |= kLargeFlag;
      return;
    }
    // No reallocation; src may overlap our buffer (e.g. assign(c_str()+5, 10))
    // so use memmove rather than memcpy.
    memmove(get_data(), str, len * sizeof(character_type));
    set_size(len);
    ensure_null_terminated();
  }
  void assign(const character_type* start, const character_type* end) {
    assign(start, static_cast<size_type>(end - start));
  }

  void assign(const character_type* str) { assign(str, base::CountStringLength(str)); }

  void assign(const BasicBaseString& other) { assign(other.get_data(), other.get_size()); }

  // Any other string-like (StringRef, SmallString, a foreign string type).
  template <typename TOther>
    requires(base::HasStringTraits<TOther, value_type> && !base::is_same_v<TOther, BasicBaseString>)
  void assign(const TOther& other) {
    assign(other.data(), static_cast<size_type>(other.size()));
  }

  template <typename TOther>
    requires(base::HasStringTraits<TOther, value_type> && !base::is_same_v<TOther, BasicBaseString>)
  BasicBaseString& operator=(const TOther& other) {
    assign(other.data(), static_cast<size_type>(other.size()));
    return *this;
  }

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
  bool is_inline() const noexcept { return !is_large(); }

  // Reserve exactly `new_capacity` (no geometric padding) — honors the
  // user's intent. Auto-growing modifiers go through grow_to_at_least()
  // instead, which uses 1.5x growth.
  void reserve(size_type new_capacity) {
    if (new_capacity > get_capacity()) realloc_to(new_capacity);
  }

  void resize(size_type new_size) {
    const size_type old_size = get_size();
    if (new_size > old_size) {
      grow_to_at_least(new_size);
      memset(get_data() + old_size, 0,
             (new_size - old_size) * sizeof(character_type));
    }
    set_size(new_size);
    ensure_null_terminated();
  }

  void clear() {
    deallocate_large();
    init_empty();
  }

  void shrink_to_fit() {
    if (!is_large() || get_size() == get_capacity()) return;

    const size_type current_size = get_size();
    if (current_size <= kSmallCapacity) {
      // Heap → inline. Snapshot before clobbering the union.
      character_type buf[kSmallCapacity + 1];
      character_type* old_data = large_.data_;
      const size_type old_capacity = get_capacity();
      if (current_size > 0) {
        memcpy(buf, old_data, current_size * sizeof(character_type));
      }
      init_empty();
      memcpy(small_.data_, buf, current_size * sizeof(character_type));
      flag_byte() = static_cast<unsigned char>(current_size & ~kLargeFlag);
      ensure_null_terminated();
      TAllocator::Free(old_data, (old_capacity + 1) * sizeof(character_type));
    } else {
      // Shrink the heap buffer to exactly current_size.
      character_type* new_data = static_cast<character_type*>(
          TAllocator::Allocate((current_size + 1) * sizeof(character_type)));
      memcpy(new_data, large_.data_, current_size * sizeof(character_type));
      new_data[current_size] = character_type{};
      deallocate_large();
      large_.data_ = new_data;
      large_.size_ = current_size;
      large_.capacity_ = current_size;
      flag_byte() |= kLargeFlag;
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
    if (count == 0) return;
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    if (new_size > get_capacity()) {
      // Source-aliasing safe + geometric growth: allocate new, copy old +
      // src (both still valid), then free old.
      const size_type new_capacity = grow_capacity(new_size);
      character_type* new_data = static_cast<character_type*>(
          TAllocator::Allocate((new_capacity + 1) * sizeof(character_type)));
      if (old_size > 0) {
        memcpy(new_data, get_data(), old_size * sizeof(character_type));
      }
      memcpy(new_data + old_size, str, count * sizeof(character_type));
      new_data[new_size] = character_type{};
      deallocate_large();
      large_.data_ = new_data;
      large_.size_ = new_size;
      large_.capacity_ = new_capacity;
      flag_byte() |= kLargeFlag;
      return;
    }
    // No realloc — dest range is past old_size so it can't overlap any
    // in-buffer src range, plain memcpy is fine.
    memcpy(get_data() + old_size, str, count * sizeof(character_type));
    set_size(new_size);
    ensure_null_terminated();
  }
  void append(const character_type* str) { append(str, base::CountStringLength(str)); }

  void append(const BasicBaseString& other) { append(other.get_data(), other.get_size()); }

  // `count` copies of `c`, for padding and fill.
  void append(size_type count, character_type c) {
    if (count == 0) return;
    const size_type old_size = get_size();
    grow_to_at_least(old_size + count);
    character_type* d = get_data();
    for (size_type i = 0; i < count; ++i) d[old_size + i] = c;
    set_size(old_size + count);
    ensure_null_terminated();
  }

  template <typename TView>
    requires(base::StringViewLike<TView, character_type> &&
             !base::is_same_v<TView, BasicBaseString>)
  void append(const TView& view) {
    append(view.data(), static_cast<size_type>(view.size()));
  }

  void pop_back() {
    BASE_DCHECK(!empty(), "Cannot pop_back an empty string");
    set_size(get_size() - 1);
    ensure_null_terminated();
  }

  void push_back(character_type c) {
    const size_type old_size = get_size();
    if (old_size == get_capacity()) grow_to_at_least(old_size + 1);
    get_data()[old_size] = c;
    set_size(old_size + 1);
    ensure_null_terminated();
  }

  void insert(size_type pos, size_type count, character_type c) {
    BASE_BUGCHECK(pos <= get_size(), "Invalid position");
    if (count == 0) return;
    const size_type old_size = get_size();
    const size_type new_size = old_size + count;
    grow_to_at_least(new_size);
    character_type* d = get_data();
    memmove(d + pos + count, d + pos, (old_size - pos) * sizeof(character_type));
    // Scalar loop — memset would only write the low byte of c.
    for (size_type i = 0; i < count; ++i) d[pos + i] = c;
    set_size(new_size);
    ensure_null_terminated();
  }

  void erase(size_type pos = 0, size_type count = npos) {
    const size_type current_size = get_size();
    BASE_BUGCHECK(pos <= current_size, "Invalid position");
    count = base::Min(count, current_size - pos);
    if (count == 0) return;

    character_type* d = get_data();
    memmove(d + pos, d + pos + count,
            (current_size - pos - count) * sizeof(character_type));
    set_size(current_size - count);
    ensure_null_terminated();
  }
  void erase(const character_type* p) {
    const size_type pos = static_cast<size_type>(p - get_data());
    BASE_BUGCHECK(pos < get_size(), "Pointer out of bounds");
    erase(pos, 1);
  }
  void erase(const character_type* start, const character_type* end) {
    BASE_BUGCHECK(start < end, "Invalid range");
    const size_type pos = static_cast<size_type>(start - get_data());
    BASE_BUGCHECK(pos < get_size(), "Pointer out of bounds");
    erase(pos, static_cast<size_type>(end - start));
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
    count = base::Min(count, current_size - pos);
    return BasicBaseString(get_data() + pos, count);
  }

  // -- Search and Compare --

  int compare(const BasicBaseString& other) const noexcept {
    const size_type left_size = get_size();
    const size_type right_size = other.get_size();
    const size_type min_size = base::Min(left_size, right_size);
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
    const size_type min_size = base::Min(left_size, right_size);
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
    const size_type min_size = base::Min(left_size, count);
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
  // Compares the `count` characters at `offset` with the first `count`
  // characters of `str`. Characters past the window don't participate, so a
  // match inside a longer string reports equal.
  int compare(size_type offset,
              size_type count,
              const character_type* str) const noexcept {
    // offset == size() is a valid empty window, like std::string::compare.
    BASE_BUGCHECK(offset <= get_size(), "Offset out of bounds");
    const size_type left_size = get_size() - offset;
    const size_type min_size = base::Min(left_size, count);
    int result = memcmp(get_data() + offset, str, min_size * sizeof(character_type));
    if (result != 0)
      return result;
    if (left_size < count)
      return -1;
    return 0;
  }

  int compare(size_type pos, size_type len, const BasicBaseString& str) const noexcept {
    BASE_BUGCHECK(pos <= get_size(), "Position out of bounds");

    const size_type rlen = base::Min(len, get_size() - pos);
    const size_type other_len = str.get_size();
    const size_type min_len = base::Min(rlen, other_len);

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

  // Substring search (overflow-safe).
  size_type find(const character_type* s, size_type pos = 0) const {
    if (!s) return npos;
    const size_type cur = get_size();
    const size_type s_len = base::CountStringLength(s);
    if (s_len == 0) return pos <= cur ? pos : npos;
    if (s_len > cur) return npos;
    if (pos > cur - s_len) return npos;
    const character_type* d = get_data();
    for (size_type i = pos; i <= cur - s_len; ++i) {
      if (memcmp(d + i, s, s_len * sizeof(character_type)) == 0)
        return i;
    }
    return npos;
  }

  size_type find(const BasicBaseString& s, size_type pos = 0) const {
    return find(s.c_str(), pos);
  }

  // First position at or after `pos` holding a character in / not in `set`.
  size_type find_first_of(const character_type* set, size_type pos = 0) const {
    if (!set) return npos;
    const size_type set_len = base::CountStringLength(set);
    for (size_type i = pos; i < get_size(); ++i) {
      if (base::find(set, set + set_len, get_data()[i]) != set + set_len) return i;
    }
    return npos;
  }

  size_type find_first_not_of(const character_type* set, size_type pos = 0) const {
    if (!set) return npos;
    const size_type set_len = base::CountStringLength(set);
    for (size_type i = pos; i < get_size(); ++i) {
      if (base::find(set, set + set_len, get_data()[i]) == set + set_len) return i;
    }
    return npos;
  }

  size_type find_first_not_of(character_type c, size_type pos = 0) const {
    for (size_type i = pos; i < get_size(); ++i) {
      if (get_data()[i] != c) return i;
    }
    return npos;
  }

  size_type find_last_not_of(const character_type* set, size_type pos = npos) const {
    if (!set || get_size() == 0) return npos;
    const size_type set_len = base::CountStringLength(set);
    size_type i = (pos == npos || pos >= get_size()) ? get_size() - 1 : pos;
    for (;; --i) {
      if (base::find(set, set + set_len, get_data()[i]) == set + set_len) return i;
      if (i == 0) return npos;
    }
  }

  size_type find_last_not_of(character_type c, size_type pos = npos) const {
    if (get_size() == 0) return npos;
    size_type i = (pos == npos || pos >= get_size()) ? get_size() - 1 : pos;
    for (;; --i) {
      if (get_data()[i] != c) return i;
      if (i == 0) return npos;
    }
  }

  bool contains(character_type c) const { return find(c) != npos; }
  bool contains(const character_type* s) const { return find(s) != npos; }
  bool contains(const BasicBaseString& s) const { return find(s) != npos; }

  bool starts_with(character_type c) const { return !empty() && get_data()[0] == c; }

  bool starts_with(const character_type* s) const {
    if (!s) return false;
    const size_type s_len = base::CountStringLength(s);
    if (s_len > get_size()) return false;
    return memcmp(get_data(), s, s_len * sizeof(character_type)) == 0;
  }

  bool starts_with(const BasicBaseString& s) const {
    if (s.size() > get_size()) return false;
    return memcmp(get_data(), s.data(), s.byte_size()) == 0;
  }

  bool ends_with(character_type c) const { return !empty() && back() == c; }

  bool ends_with(const character_type* s) const {
    if (!s) return false;
    const size_type s_len = base::CountStringLength(s);
    if (s_len > get_size()) return false;
    return memcmp(get_data() + (get_size() - s_len), s, s_len * sizeof(character_type)) == 0;
  }

  bool ends_with(const BasicBaseString& s) const {
    if (s.size() > get_size()) return false;
    return memcmp(get_data() + (get_size() - s.size()), s.data(), s.byte_size()) == 0;
  }

  // Last occurrence at or before `pos`, mirroring find()'s substring search.
  size_type rfind(character_type c, size_type pos = npos) const {
    return find_last_of(c, pos);
  }

  size_type rfind(const character_type* s, size_type pos = npos) const {
    if (!s) return npos;
    const size_type cur = get_size();
    const size_type s_len = base::CountStringLength(s);
    if (s_len == 0) return pos < cur ? pos : cur;
    if (s_len > cur) return npos;
    size_type i = (pos == npos || pos > cur - s_len) ? cur - s_len : pos;
    const character_type* d = get_data();
    for (;; --i) {
      if (memcmp(d + i, s, s_len * sizeof(character_type)) == 0) return i;
      if (i == 0) return npos;
    }
  }

  size_type rfind(const BasicBaseString& s, size_type pos = npos) const {
    return rfind(s.c_str(), pos);
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

// -- Layout invariants --
// Default-instantiated BasicBaseString is always a 3-pointer footprint —
// 24 bytes on 64-bit, 12 bytes on 32-bit. If you trip one of these the
// inline-buffer math regressed; check kSmallTailBytes / kSmallCapacity.
static_assert(sizeof(BasicBaseString<char>) == 3 * sizeof(void*),
              "BasicBaseString<char> SSO regressed");
static_assert(sizeof(BasicBaseString<char8_t>) == 3 * sizeof(void*),
              "BasicBaseString<char8_t> SSO regressed");
static_assert(sizeof(BasicBaseString<char16_t>) == 3 * sizeof(void*),
              "BasicBaseString<char16_t> SSO regressed");
static_assert(sizeof(BasicBaseString<char32_t>) == 3 * sizeof(void*),
              "BasicBaseString<char32_t> SSO regressed");
static_assert(sizeof(BasicBaseString<wchar_t>) == 3 * sizeof(void*),
              "BasicBaseString<wchar_t> SSO regressed");

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
  result.append(rhs.data(), rhs.size());
  return result;
}

}  // namespace base
