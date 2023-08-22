// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "array.h"
#include <gtest/gtest.h>

namespace base {
  
TEST(ArrayTest, DefaultConstructor) {
  Array<int, 5> arr;
  EXPECT_EQ(arr.size(), 5);
}

TEST(ArrayTest, AccessElements) {
  Array<int, 3> arr;
  arr[0] = 1;
  arr[1] = 2;
  arr[2] = 3;
  EXPECT_EQ(arr[0], 1);
  EXPECT_EQ(arr.at(1), 2);
  EXPECT_EQ(arr[2], 3);
  // Add more tests to check for out-of-bounds if needed
}

TEST(ArrayTest, Fill) {
  Array<int, 3> arr;
  arr.fill(42);
  EXPECT_EQ(arr[0], 42);
  EXPECT_EQ(arr[1], 42);
  EXPECT_EQ(arr[2], 42);
}

TEST(ArrayTest, DataAccessors) {
  Array<int, 3> arr;
  arr[0] = 10;
  arr[1] = 20;
  arr[2] = 30;
  EXPECT_EQ(*arr.data(), 10);
  EXPECT_EQ(*arr.begin(), 10);
  EXPECT_EQ(*arr.front(), 10);
  EXPECT_EQ(*arr.back(), 30);
}

} // namespace base