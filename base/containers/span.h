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

namespace base {
template <typename T>
class Span {
 public:
  explicit constexpr Span(const T* ptr, mem_size len) : ptr_(ptr), len_(len) {}

  // TODO: this allows to create a span from a span, which is not intended
  // behaviour, disable that
  template <class TT>
  constexpr Span(TT& container) noexcept
    requires HasContainerTraits<TT>
      : Span(container.data(), container.size()) {}

  template <mem_size N>
  constexpr Span(T (&a)[N]) noexcept  // NOLINT(runtime/explicit)
      : Span(a, N) {}

  const T* data() const noexcept { return ptr_; }
  mem_size size() const noexcept { return len_; }
  mem_size length() const noexcept { return len_; }
  bool empty() const noexcept { return ptr_ == nullptr; }

#if 0
  template <TRhs>
  T& operator=(const TRhs& rhs) requires HasContainerTraits<TRhs> {
    return *this;
  }
#endif

  // FIXME(Vince): we sacrifice const for dcheck here atm
  inline CONSTEXPR_ND const T& operator[](mem_size index) const noexcept {
    DCHECK(index < len_);
    return ptr_[index];
  }

  CONSTEXPR_ND T& front() noexcept {
    DCHECK(ptr_ && len_ > 0);
    return *ptr_;
  }

  CONSTEXPR_ND T& back() noexcept {
    DCHECK(ptr_ && len_ > 0);
    return *ptr_[len_ - 1];
  }

  // iterator to beginning
  CONSTEXPR_ND T* begin() const noexcept {
    return const_cast<T*>(ptr_);  // Remove const_cast if T* is non-const
  }
  // iterator to end
  CONSTEXPR_ND T* end() const noexcept {
    return const_cast<T*>(ptr_ + len_);  // Remove const_cast if T* is non-const
  }
  // For const iteration
  CONSTEXPR_ND const T* cbegin() const noexcept { return ptr_; }
  CONSTEXPR_ND const T* cend() const noexcept { return ptr_ + len_; }

 private:
  const T* ptr_;
  mem_size len_;
};

// adapter for containers.
template <class TContainer>
auto MakeSpan(TContainer& container)
  requires HasContainerTraits<TContainer>
{
  return base::Span(container.data(), container.size());
}
}  // namespace base