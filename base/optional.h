// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>

#include <new>

namespace base {

template <typename T>
class Optional {
 public:
  Optional() noexcept : is_empty_(true) {}

  Optional(const T& value) : is_empty_(false) {
    ::new (&storage_[0]) T(value);
  }

  Optional(T&& value) : is_empty_(false) {
    ::new (&storage_[0]) T(base::move(value));
  }

  Optional(const Optional& other) : is_empty_(other.is_empty_) {
    if (!is_empty_)
      ::new (&storage_[0]) T(*other.storage());
  }

  Optional(Optional&& other) noexcept : is_empty_(other.is_empty_) {
    if (!is_empty_) {
      ::new (&storage_[0]) T(base::move(*other.storage()));
      other.reset();
    }
  }

  ~Optional() {
    if (!is_empty_)
      storage()->~T();
  }

  Optional& operator=(const Optional& other) {
    if (this != &other) {
      reset();
      if (!other.is_empty_) {
        ::new (&storage_[0]) T(*other.storage());
        is_empty_ = false;
      }
    }
    return *this;
  }

  Optional& operator=(Optional&& other) noexcept {
    if (this != &other) {
      reset();
      if (!other.is_empty_) {
        ::new (&storage_[0]) T(base::move(*other.storage()));
        is_empty_ = false;
        other.reset();
      }
    }
    return *this;
  }

  Optional& operator=(const T& value) {
    reset();
    ::new (&storage_[0]) T(value);
    is_empty_ = false;
    return *this;
  }

  Optional& operator=(T&& value) {
    reset();
    ::new (&storage_[0]) T(base::move(value));
    is_empty_ = false;
    return *this;
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    reset();
    ::new (&storage_[0]) T(base::forward<Args>(args)...);
    is_empty_ = false;
  }

  void reset() {
    if (!is_empty_) {
      storage()->~T();
      is_empty_ = true;
    }
  }

  bool has_value() const noexcept { return !is_empty_; }
  explicit operator bool() const noexcept { return !is_empty_; }

  T& value() noexcept {
    BASE_DCHECK(!is_empty_, "base::Optional::value(): tried to access empty value");
    return *storage();
  }

  const T& value() const noexcept {
    BASE_DCHECK(!is_empty_, "base::Optional::value(): tried to access empty value");
    return *storage();
  }

  T& operator*() noexcept { return value(); }
  const T& operator*() const noexcept { return value(); }

  T* operator->() noexcept {
    BASE_DCHECK(!is_empty_);
    return storage();
  }
  const T* operator->() const noexcept {
    BASE_DCHECK(!is_empty_);
    return storage();
  }

  T value_or(const T& default_value) const {
    return is_empty_ ? default_value : *storage();
  }

 private:
  T* storage() { return reinterpret_cast<T*>(&storage_[0]); }
  const T* storage() const { return reinterpret_cast<const T*>(&storage_[0]); }

 private:
  alignas(T) byte storage_[sizeof(T)]{};
  bool is_empty_;
};
}  // namespace base
