// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// The registry roots, defined once in base so every DLL shares them
// (base/containers/init_chain.h explains why PE needs this).

#include <base/feature.h>
#include <base/option.h>

namespace base {

InitChain<Feature>*& Feature::ChainRoot() noexcept {
  static InitChain<Feature>* root{nullptr};
  return root;
}

InitChain<OptionBase>*& OptionBase::ChainRoot() noexcept {
  static InitChain<OptionBase>* root{nullptr};
  return root;
}

}  // namespace base
