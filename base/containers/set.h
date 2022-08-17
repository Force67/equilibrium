// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/red_black_tree.h>

namespace base {

template <typename T>
class Set final : public RedBlackTree<T> {
 public:
};
}  // namespace base
