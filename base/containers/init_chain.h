// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Static list that can be executed once on startup.
// Guarantees good assembly quality.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>

namespace base {
template <typename TItem>
class InitChain {
 public:
  constexpr InitChain() noexcept = default;

  // Puts |owner| on the chain. Call this from the derived constructor's *body*
  // rather than a mem-initializer: the chain is globally reachable, and until
  // the derived constructor finishes there is nothing complete for a VisitAll
  // to hand to its functor. Taking `this` in a base-class mem-initializer also
  // hands a pointer-to-uninitialized-object across a function boundary, which
  // is what -Wmaybe-uninitialized reports.
  STRONG_INLINE void Register(const TItem* owner) noexcept {
    Register(root(), owner);
  }
  STRONG_INLINE void Register(InitChain*& parent, const TItem* owner) noexcept {
    owner_ = owner;
    next_ = parent;
    parent = this;
  }

  // Disable copy
  InitChain(const InitChain&) = delete;
  InitChain(InitChain&&) = delete;

  STRONG_INLINE static InitChain*& root() noexcept {
    static InitChain* root{nullptr};
    return root;
  }

  template <typename TFunctor>
  STRONG_INLINE static mem_size VisitAll(const TFunctor functor,
                                         InitChain*& start = root(),
                                         bool clear = false) {
    mem_size total = 0;

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