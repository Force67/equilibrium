// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/deque.h>
#include <base/containers/vector.h>
#include <base/memory/unique_pointer.h>

namespace {

TEST(SimpleDequeTest, StartsEmpty) {
  base::SimpleDeque<i32> queue;

  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0u);
}

TEST(SimpleDequeTest, PushBackAndPopFrontIsFifo) {
  base::SimpleDeque<i32> queue;
  queue.push_back(1);
  queue.push_back(2);
  queue.push_back(3);

  EXPECT_EQ(queue.size(), 3u);
  EXPECT_EQ(queue.front(), 1);
  queue.pop_front();
  EXPECT_EQ(queue.front(), 2);
  queue.pop_front();
  EXPECT_EQ(queue.front(), 3);
  queue.pop_front();
  EXPECT_TRUE(queue.empty());
}

TEST(SimpleDequeTest, PushFrontPrepends) {
  base::SimpleDeque<i32> queue;
  queue.push_back(2);
  queue.push_front(1);

  EXPECT_EQ(queue.front(), 1);
  EXPECT_EQ(queue.back(), 2);
}

TEST(SimpleDequeTest, PopBackRemovesTheTail) {
  base::SimpleDeque<i32> queue;
  queue.push_back(1);
  queue.push_back(2);

  queue.pop_back();

  EXPECT_EQ(queue.size(), 1u);
  EXPECT_EQ(queue.back(), 1);
}

TEST(SimpleDequeTest, GrowsAndKeepsOrderAcrossTheWrap) {
  // Push and pop unevenly so the ring wraps before it has to grow.
  base::SimpleDeque<i32> queue;
  for (i32 i = 0; i < 3; ++i) queue.push_back(i);
  for (i32 i = 0; i < 2; ++i) queue.pop_front();
  for (i32 i = 3; i < 40; ++i) queue.push_back(i);

  EXPECT_EQ(queue.size(), 38u);
  for (i32 i = 2; i < 40; ++i) {
    EXPECT_EQ(queue.front(), i);
    queue.pop_front();
  }
  EXPECT_TRUE(queue.empty());
}

TEST(SimpleDequeTest, IteratesFrontToBack) {
  base::SimpleDeque<i32> queue;
  queue.push_back(2);
  queue.push_back(3);
  queue.push_front(1);

  base::Vector<i32> seen;
  for (i32 v : queue) seen.push_back(v);

  ASSERT_EQ(seen.size(), 3u);
  EXPECT_EQ(seen[0], 1);
  EXPECT_EQ(seen[1], 2);
  EXPECT_EQ(seen[2], 3);
}

TEST(SimpleDequeTest, IteratesWhenConst) {
  base::SimpleDeque<i32> queue;
  queue.push_back(5);
  const auto& const_queue = queue;

  mem_size seen = 0;
  for (i32 v : const_queue) {
    EXPECT_EQ(v, 5);
    ++seen;
  }
  EXPECT_EQ(seen, 1u);
}

TEST(SimpleDequeTest, IterationOverAnEmptyDequeDoesNothing) {
  base::SimpleDeque<i32> queue;
  EXPECT_TRUE(queue.begin() == queue.end());
}

TEST(SimpleDequeTest, HoldsMoveOnlyElements) {
  base::SimpleDeque<base::UniquePointer<i32>> queue;

  queue.push_back(base::MakeUnique<i32>(7));
  queue.push_front(base::MakeUnique<i32>(3));

  EXPECT_EQ(queue.size(), 2u);
  EXPECT_EQ(*queue.front(), 3);
  EXPECT_EQ(*queue.back(), 7);
}

TEST(SimpleDequeTest, EmplaceBack) {
  base::SimpleDeque<i32> queue;

  queue.emplace_back(4);

  EXPECT_EQ(queue.size(), 1u);
  EXPECT_EQ(queue.back(), 4);
}

}  // namespace
