// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// A span provides a safe way to iterate over and index into objects that are
// arranged back-to-back in memory. Such as objects stored in a built-in array,
// base::array , or base::Vector . If you typically access a sequence of
// back-to-back objects using a pointer and an index, a span is a safer,
// lightweight alternative.

#pragma once

#include <base/arch.h>
#include <base/check.h>

#include <base/containers/container_traits.h>
#include <base/meta/traits.h>

namespace base {
template <typename T>
class Span {
 public:
  static constexpr mem_size npos = static_cast<mem_size>(-1);

  constexpr Span() noexcept : ptr_(nullptr), len_(0) {}
  explicit constexpr Span(const T* ptr, mem_size len) : ptr_(ptr), len_(len) {}

  // TODO: disallow creating a span from a span.
  template <class TT>
  constexpr Span(TT& container) noexcept
    requires HasContainerTraits<TT>
      : Span(container.data(), container.size()) {}

  template <mem_size N>
  constexpr Span(T (&a)[N]) noexcept  // NOLINT(runtime/explicit)
      : Span(a, N) {}

  // Span<T> -> Span<const T>, also from a temporary, which the container
  // constructor above cannot bind. The array-pointer test admits added
  // qualifiers only, never a derived-to-base step that would change stride.
  template <typename U>
    requires(!base::is_same_v<U, T> && base::is_convertible_v<U (*)[], T (*)[]>)
  constexpr Span(const Span<U>& other) noexcept  // NOLINT(runtime/explicit)
      : Span(other.data(), other.size()) {}

  // Elements are as mutable as T: a Span<u8> is a writable window (File reads
  // into one), a Span<const u8> is not. begin() has always handed out T*.
  T* data() const noexcept { return const_cast<T*>(ptr_); }
  mem_size size() const noexcept { return len_; }
  mem_size length() const noexcept { return len_; }
  mem_size size_bytes() const noexcept { return len_ * sizeof(T); }
  bool empty() const noexcept { return len_ == 0; }

  // Out-of-range bounds are a programmer error, as in operator[].
  Span first(mem_size count) const noexcept {
    BASE_DCHECK(count <= len_);
    return Span(ptr_, count);
  }
  Span last(mem_size count) const noexcept {
    BASE_DCHECK(count <= len_);
    return Span(ptr_ + (len_ - count), count);
  }
  Span subspan(mem_size offset, mem_size count = npos) const noexcept {
    BASE_DCHECK(offset <= len_);
    if (count == npos) count = len_ - offset;
    BASE_DCHECK(count <= len_ - offset);
    return Span(ptr_ + offset, count);
  }

#if 0
  template <TRhs>
  T& operator=(const TRhs& rhs) requires HasContainerTraits<TRhs> {
    return *this;
  }
#endif

  inline BASE_CONSTEXPR_ND T& operator[](mem_size index) const noexcept {
    BASE_DCHECK(index < len_);
    return data()[index];
  }

  BASE_CONSTEXPR_ND T& front() const noexcept {
    BASE_DCHECK(ptr_ && len_ > 0);
    return *data();
  }

  BASE_CONSTEXPR_ND T& back() const noexcept {
    BASE_DCHECK(ptr_ && len_ > 0);
    return data()[len_ - 1];
  }

  // iterator to beginning
  BASE_CONSTEXPR_ND T* begin() const noexcept {
    return const_cast<T*>(ptr_);  // Remove const_cast if T* is non-const
  }
  // iterator to end
  BASE_CONSTEXPR_ND T* end() const noexcept {
    return const_cast<T*>(ptr_ + len_);  // Remove const_cast if T* is non-const
  }
  // For const iteration
  BASE_CONSTEXPR_ND const T* cbegin() const noexcept { return ptr_; }
  BASE_CONSTEXPR_ND const T* cend() const noexcept { return ptr_ + len_; }

 private:
  const T* ptr_;
  mem_size len_;
};

namespace span_detail {
template <typename P>
struct ElementOf;
template <typename T>
struct ElementOf<T*> {
  using type = T;
};
}  // namespace span_detail

// base::Span(container) spans the container's element type, const when the
// container is.
template <class TT>
  requires HasContainerTraits<TT>
Span(TT&) -> Span<typename span_detail::ElementOf<
    decltype(static_cast<TT*>(nullptr)->data())>::type>;

// adapter for containers.
template <class TContainer>
auto MakeSpan(TContainer& container)
  requires HasContainerTraits<TContainer>
{
  return base::Span(container.data(), container.size());
}
}  // namespace base