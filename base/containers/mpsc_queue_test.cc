#include <gtest/gtest.h>
#include "mpsc_queue.h"

class MpscQueueTest : public ::testing::Test {
 protected:
  base::MpscQueue<int> queue{10};  // Example with a queue of ints
};

TEST_F(MpscQueueTest, IsEmptyOnCreation) {
  EXPECT_TRUE(queue.empty());
}

TEST_F(MpscQueueTest, EnqueueSingleItem) {
  EXPECT_TRUE(queue.enqueue(1));
  EXPECT_EQ(queue.count(), 1);
}

TEST_F(MpscQueueTest, DequeueSingleItem) {
  queue.enqueue(1);
  EXPECT_EQ(queue.dequeue(), 1);
  EXPECT_TRUE(queue.empty());
}

TEST_F(MpscQueueTest, EnqueueUntilFull) {
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(queue.enqueue(i));
  }
  EXPECT_FALSE(queue.enqueue(11));  // Queue should be full now
  EXPECT_EQ(queue.count(), 10);
}

TEST_F(MpscQueueTest, DequeueUntilEmpty) {
  for (int i = 0; i < 10; ++i) {
    queue.enqueue(i);
  }
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(queue.dequeue(), i);
  }
  EXPECT_TRUE(queue.empty());
}