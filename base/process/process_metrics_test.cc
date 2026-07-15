// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <gtest/gtest.h>

namespace base {
namespace {

TEST(ProcessMetricsTest, ReportsCurrentProcessResidentSet) {
  ProcessMemoryUsage usage;
  ASSERT_TRUE(QueryCurrentProcessMemoryUsage(&usage));
  EXPECT_GT(usage.resident_set_bytes, 0u);
  if (usage.peak_resident_set_bytes > 0) {
    EXPECT_GE(usage.peak_resident_set_bytes, usage.resident_set_bytes);
  }
}

TEST(ProcessMetricsTest, RejectsNullOutput) {
  EXPECT_FALSE(QueryCurrentProcessMemoryUsage(nullptr));
}

}  // namespace
}  // namespace base
