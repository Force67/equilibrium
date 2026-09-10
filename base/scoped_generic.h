// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SCOPED_GENERIC_H_
#define BASE_SCOPED_GENERIC_H_

#include <stdlib.h>

#include "base/check.h"
#include "base/memory/move.h"
#include "base/meta/traits.h"

namespace base {

// Like unique_ptr with a custom deleter, but holds a value instead of a
// pointer and treats a trait-defined "invalid" value as empty. Intended for
// non-pointer resources such as file descriptors and OS handles.
//
// Traits must provide InvalidValue() and Free(). Traits may also extend
// ScopedGenericOwnershipTracking and implement Acquire/Release to observe
// ownership transfers.
//
//   struct FooScopedTraits {
//     static int InvalidValue() { return 0; }
//     static void Free(int f) { ::FreeFoo(f); }  // Not called when f is invalid.
//   };
//   using ScopedFoo = ScopedGeneric<int, FooScopedTraits>;
struct ScopedGenericOwnershipTracking {};

template <typename T, typename Traits>
class ScopedGeneric {
 private:
  // Empty base class optimization: keeps a D member with no space overhead
  // when D is an empty class.
  struct Data : public Traits {
    explicit Data(const T& in) : generic(in) {}
    Data(const T& in, const Traits& other) : Traits(other), generic(in) {}
    T generic;
  };

 public:
  typedef T element_type;
  typedef Traits traits_type;

  ScopedGeneric() : data_(traits_type::InvalidValue()) {}

  // Takes ownership of the resource held by |value|.
  explicit ScopedGeneric(const element_type& value) : data_(value) {
    TrackAcquire(data_.generic);
  }

  // Initializes with a stateful traits object.
  ScopedGeneric(const element_type& value, const traits_type& traits)
      : data_(value, traits) {
    TrackAcquire(data_.generic);
  }

  // Move constructor.
  ScopedGeneric(ScopedGeneric<T, Traits>&& rvalue)
      : data_(rvalue.release(), rvalue.get_traits()) {
    TrackAcquire(data_.generic);
  }
  ScopedGeneric(const ScopedGeneric&) = delete;
  ScopedGeneric& operator=(const ScopedGeneric&) = delete;

  virtual ~ScopedGeneric() { FreeIfNecessary(); }

  ScopedGeneric& operator=(ScopedGeneric<T, Traits>&& rvalue) {
    reset(rvalue.release());
    return *this;
  }

  // Frees the currently owned object, if any, then takes ownership of the
  // new value. Self-reset is not allowed.
  void reset(const element_type& value = traits_type::InvalidValue()) {
    if (data_.generic != traits_type::InvalidValue() && data_.generic == value)
      abort();
    FreeIfNecessary();
    data_.generic = value;
    TrackAcquire(value);
  }

  void swap(ScopedGeneric& other) {
    if (&other == this) {
      return;
    }

    TrackRelease(data_.generic);
    other.TrackRelease(other.data_.generic);

    base::swap(static_cast<Traits&>(data_), static_cast<Traits&>(other.data_));
    base::swap(data_.generic, other.data_.generic);

    TrackAcquire(data_.generic);
    other.TrackAcquire(other.data_.generic);
  }

  // Releases the object and returns it. This object then holds the invalid
  // value and no longer owns the resource.
  element_type release() {
    element_type old_generic = data_.generic;
    data_.generic = traits_type::InvalidValue();
    TrackRelease(old_generic);
    return old_generic;
  }

  // Receiver hands out a T* for taking ownership via out-parameter. On
  // destruction it calls reset() with the received value.
  //
  //   ScopedFoo foo;
  //   bool result = GetFoo(ScopedFoo::Receiver(foo).get());
  //
  // The Receiver lives until the end of the statement. If the value is
  // written asynchronously, keep the Receiver in an explicit scope.
  class Receiver {
   public:
    explicit Receiver(ScopedGeneric& parent) : scoped_generic_(&parent) {
      scoped_generic_->receiving_ = true;
    }
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver(Receiver&& move) {
      scoped_generic_ = move.scoped_generic_;
      move.scoped_generic_ = nullptr;
    }

    Receiver& operator=(Receiver&& move) {
      scoped_generic_ = move.scoped_generic_;
      move.scoped_generic_ = nullptr;
    }
    ~Receiver() {
      if (scoped_generic_) {
        scoped_generic_->reset(value_);
        scoped_generic_->receiving_ = false;
      }
    }
    // Points at Receiver's own storage, not ScopedGeneric's, so misuse
    // after scope exit is detectable by ASan or static analysis.
    T* get() {
      used_ = true;
      return &value_;
    }

   private:
    T value_ = Traits::InvalidValue();
    ScopedGeneric* scoped_generic_;
    bool used_ = false;
  };

  const element_type& get() const { return data_.generic; }

  bool is_valid() const { return data_.generic != traits_type::InvalidValue(); }

  bool operator==(const element_type& value) const { return data_.generic == value; }
  bool operator!=(const element_type& value) const { return data_.generic != value; }

  Traits& get_traits() { return data_; }
  const Traits& get_traits() const { return data_; }

 private:
  void FreeIfNecessary() {
    if (data_.generic != traits_type::InvalidValue()) {
      TrackRelease(data_.generic);
      data_.Free(data_.generic);
      data_.generic = traits_type::InvalidValue();
    }
  }

  template <typename Void = void>
  base::enable_if_t<base::is_base_of_v<ScopedGenericOwnershipTracking, Traits>, Void>
  TrackAcquire(const T& value) {
    if (value != traits_type::InvalidValue()) {
      data_.Acquire(static_cast<const ScopedGeneric&>(*this), value);
    }
  }

  template <typename Void = void>
  base::enable_if_t<!base::is_base_of_v<ScopedGenericOwnershipTracking, Traits>, Void>
  TrackAcquire(const T& value) {}

  template <typename Void = void>
  base::enable_if_t<base::is_base_of_v<ScopedGenericOwnershipTracking, Traits>, Void>
  TrackRelease(const T& value) {
    if (value != traits_type::InvalidValue()) {
      data_.Release(static_cast<const ScopedGeneric&>(*this), value);
    }
  }

  template <typename Void = void>
  base::enable_if_t<!base::is_base_of_v<ScopedGenericOwnershipTracking, Traits>, Void>
  TrackRelease(const T& value) {}

  // Forbid comparison: the same object must never be owned by two
  // ScopedGenerics.
  template <typename T2, typename Traits2>
  bool operator==(const ScopedGeneric<T2, Traits2>& p2) const;
  template <typename T2, typename Traits2>
  bool operator!=(const ScopedGeneric<T2, Traits2>& p2) const;

  Data data_;
  bool receiving_ = false;
};

template <class T, class Traits>
void swap(const ScopedGeneric<T, Traits>& a, const ScopedGeneric<T, Traits>& b) {
  a.swap(b);
}

template <class T, class Traits>
bool operator==(const T& value, const ScopedGeneric<T, Traits>& scoped) {
  return value == scoped.get();
}

template <class T, class Traits>
bool operator!=(const T& value, const ScopedGeneric<T, Traits>& scoped) {
  return value != scoped.get();
}

}  // namespace base

#endif  // BASE_SCOPED_GENERIC_H_
