// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/allocator/allocator_primitives.h>

#include <queue>
#include <vector>

#include <base/containers/vector.h>

namespace {
using namespace base;

TEST(VectorTest, ConstructFromIntializerList) {
  base::Vector<i32> vec = {1, 2, 3, 4, 5};

  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(vec[3], 4);
  EXPECT_EQ(vec[4], 5);
}

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
  // Everything after the erased index shifts left by one.
  for (i32 i = 5; i < 9; i++) {
    EXPECT_EQ(*vec.at(i), i + 1);
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

TEST(VectorTest, Accessors) {
  base::Vector<i32> vec;

  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  for (i32 i = 0; i < 10; i++) {
    EXPECT_EQ(vec[i], i);
  }

  for (i32 i = 0; i < 10; i++) {
    EXPECT_EQ(*vec.at(i), i);
  }

  EXPECT_EQ(vec.front(), 0);
  EXPECT_EQ(vec.back(), 9);
}

TEST(VectorTest, ShrinkToFit) {
  base::Vector<i32> vec;

  for (i32 i = 0; i < 20; i++) {
    vec.push_back(i);
  }
  vec.resize(10);
  EXPECT_EQ(vec.size(), 10);
  EXPECT_NE(vec.capacity(), 10);

  vec.shrink_to_fit();
  EXPECT_EQ(vec.size(), 10);
  EXPECT_EQ(vec.capacity(), 10);
}

TEST(VectorFindTest, FindExistingElement) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i * 2);  // Fill with even numbers
  }

  auto* found = vec.find(4);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 4);
}

TEST(VectorFindTest, FindNonExistingElement) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i * 2);  // Fill with even numbers
  }

  auto* notFound = vec.find(3);  // 3 is not in the vector
  EXPECT_EQ(notFound, nullptr);
}

TEST(VectorFindTest, FindFirstElement) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  auto* found = vec.find(0);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 0);
}

TEST(VectorFindTest, FindLastElement) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  auto* found = vec.find(9);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 9);
}

TEST(VectorFindTest, FindMiddleElement) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  auto* found = vec.find(5);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 5);
}

TEST(VectorFindTest, EmptyVectorFind) {
  base::Vector<i32> vec;

  auto* notFound = vec.find(1);  // Vector is empty
  EXPECT_EQ(notFound, nullptr);
}

TEST(VectorTest, ConstructFilled) {
  base::Vector<i32> vec(4, 7);

  EXPECT_EQ(vec.size(), 4);
  for (mem_size i = 0; i < vec.size(); ++i) EXPECT_EQ(vec[i], 7);
}

TEST(VectorTest, ConstructFilledIsNotMistakenForARange) {
  // Both arguments are ints: the fill constructor must win over the range one.
  base::Vector<u8> vec(3, 0xab);

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[2], 0xab);
}

TEST(VectorTest, ConstructFromPointerRange) {
  const i32 source[] = {2, 4, 6};
  base::Vector<i32> vec(source, source + 3);

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 2);
  EXPECT_EQ(vec[1], 4);
  EXPECT_EQ(vec[2], 6);
}

TEST(VectorTest, ConstructFromInputIteratorRange) {
  // Single-pass iterator: the length is not known before the range is walked.
  std::vector<i32> source = {1, 2, 3};
  base::Vector<i32> vec(source.begin(), source.end());

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[2], 3);
}

TEST(VectorTest, ConstructFromEmptyRange) {
  const i32* null = nullptr;
  base::Vector<i32> vec(null, null);

  EXPECT_TRUE(vec.empty());
}

TEST(VectorTest, AssignFillIsNotMistakenForARange) {
  base::Vector<i32> vec;
  const u32 count = 3;

  vec.assign(count, 5);

  EXPECT_EQ(vec.size(), 3u);
  EXPECT_EQ(vec[2], 5);
}

TEST(VectorTest, AssignFromARange) {
  const i32 source[] = {1, 2};
  base::Vector<i32> vec = {9, 9, 9};

  vec.assign(source, source + 2);

  EXPECT_EQ(vec.size(), 2u);
  EXPECT_EQ(vec[0], 1);
}

TEST(VectorTest, InsertAnInitializerList) {
  base::Vector<i32> vec = {1, 4};

  vec.insert(vec.begin() + 1, {2, 3});

  ASSERT_EQ(vec.size(), 4u);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(vec[3], 4);
}

TEST(VectorTest, InsertAnInitializerListAtTheEnd) {
  base::Vector<i32> vec = {1};

  vec.insert(vec.end(), {2, 3});

  ASSERT_EQ(vec.size(), 3u);
  EXPECT_EQ(vec[2], 3);
}

TEST(VectorTest, SwapExchangesContents) {
  base::Vector<i32> a = {1, 2, 3};
  base::Vector<i32> b = {9};

  a.swap(b);

  EXPECT_EQ(a.size(), 1);
  EXPECT_EQ(a[0], 9);
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b[0], 1);
}

TEST(VectorTest, ReverseIterationWalksBackwards) {
  base::Vector<i32> vec = {1, 2, 3};

  base::Vector<i32> seen;
  for (auto it = vec.rbegin(); it != vec.rend(); ++it) seen.push_back(*it);

  ASSERT_EQ(seen.size(), 3);
  EXPECT_EQ(seen[0], 3);
  EXPECT_EQ(seen[1], 2);
  EXPECT_EQ(seen[2], 1);
}

TEST(VectorTest, ReverseIterationOverAnEmptyVectorDoesNothing) {
  base::Vector<i32> vec;
  EXPECT_TRUE(vec.rbegin() == vec.rend());
}

TEST(VectorTest, BacksAStandardContainerAdaptor) {
  // The adaptors look up size_type/reference/const_reference on the container.
  std::priority_queue<i32, base::Vector<i32>, std::greater<i32>> queue;
  queue.push(5);
  queue.push(1);
  queue.push(3);

  EXPECT_EQ(queue.top(), 1);
  queue.pop();
  EXPECT_EQ(queue.top(), 3);
}

TEST(VectorFindTest, FindInUnsortedArray) {
  base::Vector<i32> vec;
  vec.push_back(5);
  vec.push_back(1);
  vec.push_back(3);
  vec.push_back(2);
  vec.push_back(4);

  auto* found = vec.find(3);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 3);
}

}  // namespace
