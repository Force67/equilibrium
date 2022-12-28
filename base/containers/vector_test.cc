// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/allocator/allocator_primitives.h>

#include <vector>
#include <base/containers/vector.h>

namespace {
using namespace base;

struct Complex2 {
  i32 a;
  i32 b;

  Complex2() : a(1337), b(1338) {}
  Complex2(i32 a, i32 b) : a(a), b(b) {}
};

TEST(VectorTest, AddTrivial) {
  base::Vector<i32> vec(10, base::VectorReservePolicy::kForPushback);

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
}

TEST(VectorTest, AddTrivialReAlloc) {
  base::Vector<i32> vec(10, base::VectorReservePolicy::kForPushback);

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
    std::vector<Complex> vec;
    for (i32 i = 0; i < 10; i++) {
      vec.push_back(Complex(i, destruct_count));
    }
    EXPECT_EQ(vec.size(), 10);
  }

  EXPECT_EQ(destruct_count, 45);
  
  destruct_count = 0;

  {
    base::Vector<Complex> vec(10, base::VectorReservePolicy::kForPushback);
    for (i32 i = 0; i < 10; i++) {
      vec.push_back(Complex(i, destruct_count));
    }
    EXPECT_EQ(vec.size(), 10);
  }

  EXPECT_EQ(destruct_count, 20);
}

TEST(VectorTest, EraseItem) {
  base::Vector<i32> vec(10, base::VectorReservePolicy::kForPushback);

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
  base::Vector<i32> vec(10, base::VectorReservePolicy::kForPushback);

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
  base::Vector<i32> vec;

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
  for (i32 i = 0; i < 10; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }
}

TEST(VectorTest, Resize) {
  base::Vector<i32> vec;
  std::vector<i32> std_vec;

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }
  for (i32 i = 0; i < 10; i++) {
    std_vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), std_vec.size());

  vec.resize(vec.size() + 20);
  std_vec.resize(std_vec.size() + 20);

  EXPECT_EQ(vec.size(), std_vec.size());

  for (i32 i = 10; i < 30; i++) {
    EXPECT_EQ(vec[i], 0);
  }

  for (i32 i = 10; i < 30; i++) {
    EXPECT_EQ(std_vec[i], 0);
  }
}

TEST(VectorTest, ResizeComplex) {
  base::Vector<Complex2> vec;
  std::vector<Complex2> std_vec;

  for (i32 i = 0; i < 10; i++) {
    auto complex = Complex2(i, i * 2);
    vec.push_back(complex);
  }
  for (i32 i = 0; i < 10; i++) {
    auto complex = Complex2(i, i * 2);
    std_vec.push_back(complex);
  }

  EXPECT_EQ(vec.size(), std_vec.size());

  vec.resize(vec.size() + 20);
  std_vec.resize(std_vec.size() + 20);

  EXPECT_EQ(vec.size(), std_vec.size());

  for (i32 i = 10; i < 30; i++) {
    EXPECT_EQ(vec[i].a, 1337);
    EXPECT_EQ(vec[i].b, 1338);
  }

  for (i32 i = 10; i < 30; i++) {
    EXPECT_EQ(std_vec[i].a, 1337);
    EXPECT_EQ(std_vec[i].b, 1338);
  }
}

TEST(VectorTest, Clear) {
  base::Vector<Complex2> vec;
  for (i32 i = 0; i < 10; i++) {
    auto complex = Complex2(i, i * 2);
    vec.push_back(complex);
  }
  EXPECT_EQ(vec.size(), 10);
  vec.clear();
  EXPECT_EQ(vec.size(), 0);
}
}  // namespace
