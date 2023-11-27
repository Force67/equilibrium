// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "function_ref.h"
#if 0
namespace {
TEST(FunctionRef, CanInitializeWithLambda) {
  base::FunctionRef<int(int, int)> func = [](int x, int y) { return x + y; };
  EXPECT_EQ(func(2, 3), 5);
}

int Add(int x, int y) {
  return x + y;
}

TEST(FunctionRef, CanInitializeWithFreeFunction) {
  base::FunctionRef<int(int, int)> func(&Add);
  EXPECT_EQ(func(2, 3), 5);
}

#if 0
class TestClass {
 public:
  int Add(int x, int y) { return x + y; }
};

TEST(FunctionRef, CanInitializeWithMemberFunction) {
  TestClass obj;
  base::FunctionRef<int(TestClass&, int, int)> func = &TestClass::Add;
  EXPECT_EQ(func(obj, 2, 3), 5);
}
#endif

struct AddFunctor {
  int operator()(int x, int y) { return x + y; }
};

TEST(FunctionRef, CanInitializeWithFunctionObject) {
  base::FunctionRef<int(int, int)> func = AddFunctor();
  EXPECT_EQ(func(2, 3), 5);
}
}
#endif