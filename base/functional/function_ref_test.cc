// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "function_ref.h"

namespace {
TEST(FunctionRef, CanInitializeWithLambda) {
  auto add = [](int x, int y) { return x + y; };
  base::FunctionRef<int(int, int)> func = add;
  EXPECT_EQ(func(2, 3), 5);
}

TEST(FunctionRef, CanInitializeWithCapturingLambda) {
  int offset = 10;
  auto add_offset = [&offset](int x) { return x + offset; };
  base::FunctionRef<int(int)> func = add_offset;
  EXPECT_EQ(func(5), 15);
  offset = 20;
  EXPECT_EQ(func(5), 25);
}

int Add(int x, int y) {
  return x + y;
}

TEST(FunctionRef, CanInitializeWithFreeFunction) {
  base::FunctionRef<int(int, int)> func(&Add);
  EXPECT_EQ(func(2, 3), 5);
}

struct AddFunctor {
  int operator()(int x, int y) { return x + y; }
};

TEST(FunctionRef, CanInitializeWithFunctionObject) {
  AddFunctor functor;
  base::FunctionRef<int(int, int)> func = functor;
  EXPECT_EQ(func(2, 3), 5);
}

TEST(FunctionRef, VoidReturn) {
  int calls = 0;
  auto increment_calls = [&calls] { calls++; };
  base::FunctionRef<void()> func = increment_calls;
  func();
  func();
  EXPECT_EQ(calls, 2);
}

int Invoke(base::FunctionRef<int(int)> func, int value) {
  return func(value);
}

TEST(FunctionRef, CanPassAsParameter) {
  EXPECT_EQ(Invoke([](int x) { return x * 2; }, 21), 42);
}

TEST(FunctionRef, CanCopy) {
  base::FunctionRef<int(int, int)> func(&Add);
  base::FunctionRef<int(int, int)> copy = func;
  EXPECT_EQ(copy(2, 3), 5);
}
}  // namespace
