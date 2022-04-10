// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Feature flag implementation.
#pragma once

#include <arch.h>
#include <base/containers/init_chain.h>

namespace base {
struct BASE_EXPORT Feature : InitChain<Feature> {
  enum class Flags : arch_types::u32 {
    kNone = 1 << 0,
    kHidden = 1 << 1,  //< If this flag isn't set, assume public.
  };

  constexpr STRONG_INLINE Feature(const char* name, Flags f = Flags::kNone, bool enabled = true)
      : InitChain(this), name(name), flags(f), enabled(enabled) {}

  inline operator bool() const { return enabled; }

  const char* name;
  Flags flags;
  bool enabled;
};

// For concentration of flags
inline constexpr Feature::Flags operator|(Feature::Flags lhs,
                                          Feature::Flags rhs) {
  return static_cast<Feature::Flags>(static_cast<arch_types::u32>(lhs) |
                                     static_cast<arch_types::u32>(rhs));
}

// Use as follows:
// static const Feature myFeature{"Test", Feature::Flags::kHidden, true};

// access all features like:
// Feature::VisitAll([](const Feature* f) {});
}  // namespace base