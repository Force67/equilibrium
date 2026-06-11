#include <gtest/gtest.h>

#include <string>

#include "queue.h"

namespace base {
namespace {

TEST(QueueTest, StartsEmpty) {
  base::Queue<int> queue;
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(queue.size(), 0u);
}

TEST(QueueTest, FifoOrder) {
  base::Queue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  EXPECT_EQ(queue.size(), 3u);
  EXPECT_EQ(queue.front(), 1);
  EXPECT_EQ(queue.back(), 3);

  queue.pop();
  EXPECT_EQ(queue.front(), 2);
  queue.pop();
  EXPECT_EQ(queue.front(), 3);
  queue.pop();
  EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, NonTrivialElements) {
  base::Queue<std::string> queue;
  queue.push("first");
  queue.push("second");
  EXPECT_EQ(queue.front(), "first");
  queue.front() = "changed";
  EXPECT_EQ(queue.front(), "changed");
  queue.pop();
  EXPECT_EQ(queue.front(), "second");
}

TEST(QueueTest, GrowsPastInitialCapacity) {
  base::Queue<int> queue;
  for (int i = 0; i < 1000; ++i) {
    queue.push(i);
  }
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(queue.front(), i);
    queue.pop();
  }
  EXPECT_TRUE(queue.empty());
}

}  // namespace
}  // namespace base
