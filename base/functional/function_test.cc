// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "function.h"

namespace {
TEST(Function, CanInitializeWithLambda) {
  base::Function<int(int, int)> func([](int x, int y) { return x + y; });
  EXPECT_EQ(func(2, 3), 5);
}

TEST(Function, CanInitializeWithExplicitCapturingLambda) {
  int a = 10;
  base::Function<int(int)> func([a](int x) { return x + a; });
  EXPECT_EQ(func(2), 12);
}

TEST(Function, CanInitializeWithAllCapturingLambda) {
  int a = 10;
  base::Function<int(int)> func([&](int x) { return x + a; });
  EXPECT_EQ(func(2), 12);
}

int Add(int x, int y) {
  return x + y;
}

TEST(Function, CanInitializeWithFreeFunction) {
  base::Function<int(int, int)> func(&Add);
  EXPECT_EQ(func(2, 3), 5);
}

#if 0
class TestClass {
 public:
  int Add(int x, int y) { return x + y; }
};

TEST(Function, CanInitializeWithMemberFunction) {
  TestClass obj;
  base::Function<int(TestClass&, int, int)> func = &TestClass::Add;
  EXPECT_EQ(func(obj, 2, 3), 5);
}
#endif
// A closure that captured a Function by copy holds a const member; moving the
// closure moves that member as a const Function&&, which must copy the
// Function rather than wrap it in another Function.
TEST(Function, CopiesFromAConstRvalue) {
  const base::Function<int(int)> inner([](int x) { return x * 2; });
  base::Function<int(int)> copy(static_cast<const base::Function<int(int)>&&>(inner));
  EXPECT_EQ(copy(21), 42);
  EXPECT_EQ(inner(4), 8);
}

TEST(Function, MovesAClosureThatCapturedAFunction) {
  const base::Function<int(int)> hook([](int x) { return x + 1; });
  auto closure = [hook](int x) { return hook(x) * 10; };
  base::Function<int(int)> outer(base::move(closure));
  base::Function<int(int)> moved(base::move(outer));
  EXPECT_EQ(moved(1), 20);
}

}  // namespace
