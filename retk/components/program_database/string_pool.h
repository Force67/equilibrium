// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// String pool for deduplication purposes.
#pragma once

#include <set>
#include <base/strings/xstring.h>

namespace program_database {

template <typename TChar>
class StringPool {
 public:
  using Atom = const TChar*;

  template <typename...Args>
  Atom Add(Args&&... args) {
    return interned_.emplace(args...).first->c_str();
  }

 private:
  std::set<base::BasicString<TChar>> interned_;
};
}  // namespace program_database
