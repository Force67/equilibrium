// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/containers/red_black_tree.h>

namespace {
TEST(RedBlackTree, Insert) {
  base::RedBlackTree tree;

  tree.insert(1337);
  EXPECT_TRUE(tree.searchTree(1337));
}
}  // namespace
