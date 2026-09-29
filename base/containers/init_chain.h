// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Static list that can be executed once on startup.
// Guarantees good assembly quality.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>

// The chain's root is a function-local static in a class template, so every
// shared object that instantiates the template gets its own root when it is
// compiled with -fvisibility=hidden (as a component build is): an Option
// registered in one .so is then invisible to VisitAll in another. Default
// visibility makes the root one symbol per process (ELF unique binding), so
// all objects share one chain. An instantiation is only as visible as its
// template argument, so every item type (OptionBase, Feature) carries the same
// annotation.
//
// PE has no such binding: every DLL keeps its own copy of an inline static.
// There the root lives out of line, in TItem::ChainRoot(), which base defines
// once; with base built as a DLL (EQ_BASE_SHARED) every module reaches the
// same one.
#if defined(__GNUC__) || defined(__clang__)
#define BASE_SHARED_REGISTRY __attribute__((visibility("default")))
#else
#define BASE_SHARED_REGISTRY
#endif

namespace base {
template <typename TItem>
class BASE_SHARED_REGISTRY InitChain {
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

#if defined(_WIN32)
  STRONG_INLINE static InitChain*& root() noexcept { return TItem::ChainRoot(); }
#else
  STRONG_INLINE static InitChain*& root() noexcept {
    static InitChain* root{nullptr};
    return root;
  }
#endif

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