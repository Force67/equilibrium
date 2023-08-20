// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// The class template std::optional manages an optional contained value, i.e. a
// value that may or may not be present. A common use case for optional is the
// return value of a function that may fail. As opposed to other approaches,
// such as std::pair<T,bool>, optional handles expensive-to-construct objects
// well and is more readable, as the intent is expressed explicitly. Any
// instance of optional<T> at any given point in time either contains a value or
// does not contain a value. If an optional<T> contains a value, the value is
// guaranteed to be allocated as part of the optional object footprint, i.e. no
// dynamic memory allocation ever takes place. Thus, an optional object models
// an object, not a pointer, even though operator*() and operator->() are
// defined.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

template <typename T>
class Optional {
  // BASE_NOMOVE(Expected);
 public:
  inline Optional(T value) : is_empty_(false) { *storage() = value; }
  // empty
  inline Optional() : is_empty_(true) {}

  ~Optional() {
    if (!is_empty_)
      storage()->~T();
  }

  bool failed() noexcept {
#if defined(CONFIG_DEBUG)
    has_checked_validity_ = true;
#endif
    return is_empty_;
  }

  bool has_value() noexcept {
#if defined(CONFIG_DEBUG)
    has_checked_validity_ = true;
#endif
    return !is_empty_;
  }

  CONSTEXPR_ND T& value() noexcept {
    DCHECK(!is_empty_, "base::Optional::value(): tried to access empty value");
    DCHECK(has_checked_validity_,
           "Validy wasn't checked before accessing value");
    return *storage();
  }

  Optional& operator=(Optional& rhs) {
    is_empty_ = rhs.is_empty_;
    memcpy(&storage_[0], &rhs.storage_[0], sizeof(storage_));
    ::new (&storage_[0]) T(*rhs.storage());
    return *this;
  }

 private:
  T* storage() { return reinterpret_cast<T*>(&storage_[0]); }

 private:
  alignas(T) byte storage_[sizeof(T)]{};
  bool is_empty_ : 1;
#if defined(CONFIG_DEBUG)
  bool has_checked_validity_ : 1 {false};
#endif
};
}  // namespace base