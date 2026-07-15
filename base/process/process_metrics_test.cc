// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#if defined(OS_LINUX)
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <gtest/gtest.h>

namespace base {
namespace {

TEST(ProcessMetricsTest, ReportsCurrentProcessResidentSet) {
  ProcessMemoryUsage usage;
  ASSERT_TRUE(QueryProcessMemoryUsage(GetCurrentProcessHandle(), usage));
  EXPECT_GT(usage.resident_set_bytes, 0u);
  if (usage.peak_resident_set_bytes > 0) {
    EXPECT_GE(usage.peak_resident_set_bytes, usage.resident_set_bytes);
  }
}

TEST(ProcessMetricsTest, RejectsInvalidHandleAndClearsOutput) {
  ProcessMemoryUsage usage{1, 1};
  EXPECT_FALSE(QueryProcessMemoryUsage(ProcessHandle{}, usage));
  EXPECT_EQ(usage.resident_set_bytes, 0u);
  EXPECT_EQ(usage.peak_resident_set_bytes, 0u);
}

#if defined(OS_LINUX)
TEST(ProcessMetricsTest, ReportsAnotherProcessResidentSet) {
  const pid_t child = fork();
  ASSERT_NE(child, -1);
  if (child == 0) {
    pause();
    _exit(0);
  }

  ProcessMemoryUsage usage;
  const bool queried = QueryProcessMemoryUsage(static_cast<ProcessHandle>(child), usage);
  kill(child, SIGTERM);
  waitpid(child, nullptr, 0);

  ASSERT_TRUE(queried);
  EXPECT_GT(usage.resident_set_bytes, 0u);
}
#endif

}  // namespace
}  // namespace base
