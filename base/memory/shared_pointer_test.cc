// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/memory/shared_pointer.h>
#include <base/threading/thread.h>

namespace {
struct Counted {
  explicit Counted(int* alive, int v) : alive_(alive), value(v) { ++*alive_; }
  ~Counted() { --*alive_; }
  int* alive_;
  int value;
};

struct Base {
  virtual ~Base() = default;
  int tag = 1;
};
struct Derived : Base {
  Derived() { tag = 2; }
};

TEST(SharedPointer, EmptyByDefault) {
  base::SharedPointer<int> p;
  EXPECT_FALSE(p);
  EXPECT_EQ(p.get(), nullptr);
  EXPECT_EQ(p.use_count(), 0u);
  EXPECT_TRUE(p == nullptr);
}

TEST(SharedPointer, LastOwnerDestroys) {
  int alive = 0;
  {
    auto a = base::MakeShared<Counted>(&alive, 7);
    EXPECT_EQ(alive, 1);
    EXPECT_EQ(a->value, 7);
    {
      base::SharedPointer<Counted> b = a;
      EXPECT_EQ(a.use_count(), 2u);
      EXPECT_EQ(b.get(), a.get());
    }
    EXPECT_EQ(a.use_count(), 1u);
    EXPECT_EQ(alive, 1);
  }
  EXPECT_EQ(alive, 0);
}

TEST(SharedPointer, MoveLeavesSourceEmpty) {
  int alive = 0;
  auto a = base::MakeShared<Counted>(&alive, 1);
  base::SharedPointer<Counted> b = base::move(a);
  EXPECT_FALSE(a);
  EXPECT_EQ(b.use_count(), 1u);
  b.Reset();
  EXPECT_EQ(alive, 0);
}

TEST(SharedPointer, AssignmentReleasesTheOldObject) {
  int alive = 0;
  auto a = base::MakeShared<Counted>(&alive, 1);
  auto b = base::MakeShared<Counted>(&alive, 2);
  EXPECT_EQ(alive, 2);
  a = b;
  EXPECT_EQ(alive, 1);
  EXPECT_EQ(a->value, 2);
  a = a;
  EXPECT_EQ(a.use_count(), 2u);
  b = nullptr;
  EXPECT_EQ(a.use_count(), 1u);
}

TEST(SharedPointer, ConvertsToConstAndBase) {
  base::SharedPointer<const int> c = base::MakeShared<int>(5);
  EXPECT_EQ(*c, 5);
  base::SharedPointer<Base> b = base::MakeShared<Derived>();
  EXPECT_EQ(b->tag, 2);
  auto k = base::MakeShared<const int>(9);
  EXPECT_EQ(*k, 9);
}

TEST(SharedPointer, CountsAcrossThreads) {
  int alive = 0;
  auto shared = base::MakeShared<Counted>(&alive, 3);
  constexpr int kThreads = 4, kCopies = 10000;
  base::Thread* threads[kThreads];
  for (int t = 0; t < kThreads; t++)
    threads[t] = new base::Thread(
        "sp-test",
        [shared] {
          for (int i = 0; i < kCopies; i++) {
            base::SharedPointer<Counted> copy = shared;
            (void)copy;
          }
        },
        true);
  for (auto* t : threads) {
    t->Join();
    delete t;
  }
  EXPECT_EQ(shared.use_count(), 1u);
  shared.Reset();
  EXPECT_EQ(alive, 0);
}
}  // namespace
