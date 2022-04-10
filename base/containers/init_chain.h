// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Static list that can be executed once on startup.
// Guarantees good assembly quality.
#pragma once

#include <export.h>
#include "base/compiler.h"

namespace base {
template <typename TItem>
class InitChain {
 public:
  constexpr InitChain() noexcept = default;

  // Construct from data
  STRONG_INLINE constexpr InitChain(const TItem *owner) noexcept : InitChain(root(), owner) {}
  STRONG_INLINE constexpr InitChain(InitChain*& parent, const TItem *data) noexcept
      : owner_(data), next_(parent) {
    parent = this;
  }

  // Disable copy
  InitChain(const InitChain&) = delete;
  InitChain(InitChain&&) = delete;

  STRONG_INLINE static InitChain*& root() noexcept {
    static InitChain* root{nullptr};
    return root;
  }

  template<typename TFunctor>
  STRONG_INLINE static size_t VisitAll(const TFunctor functor, InitChain*& start = root(), bool clear = false) {
    size_t total = 0;

    InitChain* i = start;

    if (clear)
      start = nullptr;

    while (i) {
      functor(i->owner_);
      ++total;

      InitChain* j = i->next_;

      if (clear)
        i->next_ = nullptr;

      i = j;
    }

    return total;
  }

 private:
  const TItem* owner_{};
  InitChain* next_{nullptr};
};
}  // namespace base