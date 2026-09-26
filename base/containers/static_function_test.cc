// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/static_function.h>
#include <base/memory/move.h>

namespace {

using Fn = base::StaticFunction<int(int), 64>;

TEST(StaticFunction, CallsAStoredLambda) {
  int base_value = 3;
  Fn fn([base_value](int x) { return x + base_value; });
  EXPECT_EQ(fn(4), 7);
}

// A non-const lvalue and a const rvalue both have to take the copy
// constructor, not the converting one, which would try to store a
// StaticFunction inside itself.
TEST(StaticFunction, CopiesFromANonConstLvalue) {
  Fn fn([](int x) { return x * 3; });
  Fn copy(fn);
  EXPECT_EQ(copy(2), 6);
  EXPECT_EQ(fn(3), 9);
}

TEST(StaticFunction, CopiesFromAConstRvalue) {
  const Fn fn([](int x) { return x - 1; });
  Fn copy(static_cast<const Fn&&>(fn));
  EXPECT_EQ(copy(5), 4);
}

TEST(StaticFunction, AssignsFromAnotherStaticFunction) {
  Fn source([](int x) { return x * x; });
  Fn target;
  target = source;
  EXPECT_EQ(target(5), 25);
  Fn moved;
  moved = base::move(source);
  EXPECT_EQ(moved(4), 16);
}

}  // namespace
