// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <memory>

#include "thread.h"

namespace {
#if 0
static std::unique_ptr<base::Thread> g_thread;

class ThreadTest final : base::Thread {
 public:
 private:
  u32 Run() override { return 0; }
};

TEST(Thread, SpawnDestroy) {
  base::Thread thread("test");
  ASSERT_TRUE(thread.good());
}
#endif
}  // namespace