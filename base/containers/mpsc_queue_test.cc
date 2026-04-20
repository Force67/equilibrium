// Tests for the wait-free unbounded multi-producer / single-consumer queue.
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "mpsc_queue.h"

namespace {

TEST(MPSCQueue, IsEmptyOnCreation) {
  base::MPSCQueue<int> q;
  EXPECT_TRUE(q.empty());
  int out = 0;
  EXPECT_FALSE(q.dequeue(out));
}

TEST(MPSCQueue, EnqueueDequeueSingleItem) {
  base::MPSCQueue<int> q;
  q.enqueue(42);
  EXPECT_FALSE(q.empty());

  int out = 0;
  ASSERT_TRUE(q.dequeue(out));
  EXPECT_EQ(out, 42);
  EXPECT_TRUE(q.empty());
}

TEST(MPSCQueue, FifoOrder) {
  base::MPSCQueue<int> q;
  for (int i = 0; i < 10; ++i) q.enqueue(int{i});
  for (int i = 0; i < 10; ++i) {
    int out = -1;
    ASSERT_TRUE(q.dequeue(out));
    EXPECT_EQ(out, i);
  }
  EXPECT_TRUE(q.empty());
}

TEST(MPSCQueue, EmplaceConstructsInPlace) {
  struct Point { int x; int y; Point(int x_, int y_) : x(x_), y(y_) {} };
  base::MPSCQueue<Point> q;
  q.emplace(3, 4);

  Point p{0, 0};
  ASSERT_TRUE(q.dequeue(p));
  EXPECT_EQ(p.x, 3);
  EXPECT_EQ(p.y, 4);
}

TEST(MPSCQueue, SizeApproxAndIteration) {
  base::MPSCQueue<int> q;
  for (int i = 0; i < 5; ++i) q.enqueue(int{i});
  EXPECT_EQ(q.size_approx(), 5u);

  int sum = 0;
  for (auto it = q.begin(); it != q.end(); ++it) sum += *it;
  EXPECT_EQ(sum, 0 + 1 + 2 + 3 + 4);
}

TEST(MPSCQueue, MultipleProducersOneConsumer) {
  // Wait-free MPSC: many producers, one consumer. Verify every produced
  // value is dequeued exactly once.
  constexpr int kProducers = 8;
  constexpr int kPerProducer = 1000;

  base::MPSCQueue<int> q;
  base::Atomic<bool> go{false};

  std::vector<std::thread> producers;
  producers.reserve(kProducers);
  for (int p = 0; p < kProducers; ++p) {
    producers.emplace_back([&, p] {
      while (!go.load(base::memory_order_acquire)) {}
      const int base = p * kPerProducer;
      for (int i = 0; i < kPerProducer; ++i) q.enqueue(int{base + i});
    });
  }

  base::Atomic<int> drained{0};
  std::vector<int> seen(kProducers * kPerProducer, 0);
  std::thread consumer([&] {
    while (!go.load(base::memory_order_acquire)) {}
    int total = kProducers * kPerProducer;
    while (drained.load() < total) {
      int v;
      if (q.dequeue(v)) {
        ASSERT_GE(v, 0);
        ASSERT_LT(v, total);
        seen[v]++;
        drained.fetch_add(1);
      }
    }
  });

  go.store(true, base::memory_order_release);
  for (auto& t : producers) t.join();
  consumer.join();

  EXPECT_EQ(drained.load(), kProducers * kPerProducer);
  for (int i = 0; i < kProducers * kPerProducer; ++i) {
    EXPECT_EQ(seen[i], 1) << "value " << i << " seen " << seen[i] << " times";
  }
  EXPECT_TRUE(q.empty());
}

TEST(MPSCQueue, DestructorDrainsRemaining) {
  // Pending items at destruction must be cleaned up without leaks.
  // (Hard to detect leaks without instrumentation; we just exercise the path.)
  base::MPSCQueue<int> q;
  for (int i = 0; i < 100; ++i) q.enqueue(int{i});
  // q goes out of scope with items still in it.
}

}  // namespace
