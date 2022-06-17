// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

namespace base {
namespace detail {

// Internal adapter class for implementing base::Reversed.
template <typename T>
class ReversedAdapter {
 public:
  explicit ReversedAdapter(T& t) : t_(t) {}
  ReversedAdapter(const ReversedAdapter& ra) : t_(ra.t_) {}
  ReversedAdapter& operator=(const ReversedAdapter&) = delete;

  auto begin() const { return t_.begin(); }
  auto end() const { return t_.end(); }

 private:
  T& t_;
};

}  // namespace detail

// Reversed returns a container adapter usable in a range-based "for" statement
// for iterating a reversible container in reverse order.
//
// Example:
//
//   vector<int> v = ...;
//   for (int i : base::Reversed(v)) {
//     // iterates through v from back to front
//   }
template <typename T>
detail::ReversedAdapter<T> Reversed(T& t) {
  return detail::ReversedAdapter<T>(t);
}
}  // namespace base