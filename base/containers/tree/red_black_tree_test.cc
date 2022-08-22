// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/containers/tree/red_black_tree.h>

namespace {
#if 0
TEST(RedBlackTree, Insert) {
  base::RedBlackTree tree;

  tree.insert(1337);
  EXPECT_TRUE(tree.find(1337));

  for (int i = 0; i < 10; i++)
    tree.insert(i);

  EXPECT_TRUE(tree.find(7));

  tree.insert(8);
}
#endif
}  // namespace
