// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Like std::optional but carries an error value instead of nullopt; the
// interface is more convenient than a two-member std::variant.
#pragma once

#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

// basically llvm's error-or class, on success we return the expected value, on
// failure, we return the 'WrongValue' which for instance could be an error
// code.
template <typename TExpected, typename TError>
class Expected {
  // BASE_NOMOVE(Expected);

  static constexpr bool types_match = base::ISSame<TExpected, TError>;

 public:
  inline Expected(const Expected& rhs) : has_error_(rhs.has_error_) {
    if (has_error_)
      error_value_ = rhs.error_value_;
    else
      expected_value_ = rhs.expected_value_;
  }
  // we do not take a const qualifier in the constructor to disable implicit
  // constructions
  template <typename U = TExpected>
  inline Expected(U value)
    requires(!base::ISSame<U, TError>)
      : has_error_(false), expected_value_(base::move(value)) {}

  template <typename U = TError>
  inline Expected(U error)
    requires(!base::ISSame<U, TExpected>)
      : has_error_(true), error_value_(base::move(error)) {}

  template <typename T>
  constexpr Expected(T value, const bool is_error)
    requires(types_match)
      : has_error_(is_error) {
    if (is_error)
      error_value_ = base::move(value);  // moving here is most likely a bug
    else
      expected_value_ = base::move(value);
  }

  ~Expected() {};

  constexpr bool has_error() noexcept {
#if defined(CONFIG_DEBUG)
    has_checked_validity_ = true;
#endif
    return has_error_;
  }

  // const in order to forbid abusing the object as a storage facility
  BASE_CONSTEXPR_ND const TExpected& value() const noexcept {
    BASE_DCHECK(!has_error_, "Invalid access on error'd value");
#if defined(CONFIG_DEBUG)
    BASE_DCHECK(has_checked_validity_, "Validity wasn't checked before accessing value");
#endif
    return expected_value_;
  }

  BASE_CONSTEXPR_ND const TError& error() const noexcept {
    BASE_DCHECK(has_error_, "Invalid access on un-error'd error");
    return error_value_;
  }

  Expected& operator=(Expected& rhs) {
    has_error_ = rhs.has_error_;
    if (has_error_)
      error_value_ = rhs.error_value_;
    else
      expected_value_ = rhs.expected_value_;
    return *this;
  }

 private:
  union {
    TExpected expected_value_;
    TError error_value_;
  };
  bool has_error_ : 1;
#if defined(CONFIG_DEBUG)
  bool has_checked_validity_ : 1 {false};
#endif
};
}  // namespace base