// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/allocator/allocator_primitives.h>
#include <base/containers/vector.h>

namespace {
using namespace base;

struct VecAllocator {
  static void* Allocate(mem_size sz) { return base::Allocate(sz); }
  static void Free(void* former, mem_size former_size) { base::Free(former); }
};

TEST(VectorTest, AddTrivial) {
  base::Vector<i32, VecAllocator> vec(10);

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
}

TEST(VectorTest, AddTrivialReAlloc) {
  base::Vector<i32, VecAllocator> vec(10);

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
  EXPECT_EQ(vec.capacity(), 10);

  vec.push_back(11);
  EXPECT_EQ(vec.size(), 11);
  EXPECT_EQ(vec.capacity(), 20);
}

TEST(VectorTest, AddComplex) {
  i32 destruct_count = 0;

  struct Complex {
    explicit Complex(i32 i, i32& dcount) : i_(i), dtor_count_(dcount) {}
    ~Complex() { dtor_count_++; }

    i32 i_;
    i32& dtor_count_;
  };

  {
    base::Vector<Complex, VecAllocator> vec(10);
    for (i32 i = 0; i < 10; i++) {
      vec.push_back(Complex(i, destruct_count));
    }
    EXPECT_EQ(vec.size(), 10);
  }

  EXPECT_EQ(destruct_count, 10);
}

TEST(VectorTest, EraseItem) {
  base::Vector<i32, VecAllocator> vec(10);

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  vec.erase(5);
  EXPECT_EQ(vec.size(), 9);

  for (i32 i = 0; i < 5; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }
  for (i32 i = 6; i < 9; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }
}

TEST(VectorTest, EraseLastItem) {
  base::Vector<i32, VecAllocator> vec(10);

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  vec.erase(9);
  EXPECT_EQ(vec.size(), 9);

  for (i32 i = 0; i < 9; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }

  for (i32 i : vec) {
    EXPECT_TRUE(i < 10);
  }
}

TEST(VectorTest, VectorAddNoPrealloc) {
  base::Vector<i32, VecAllocator> vec;

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
  for (i32 i = 0; i < 10; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }
}
}  // namespace
