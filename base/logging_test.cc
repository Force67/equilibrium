// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/strings/xstring.h>
#include <gtest/gtest.h>

namespace {
class LoggingTestSuite : public ::testing::Test {
 public:
  void SetUp() override;
  void TearDown() override;

 public:
  base::String last_msg_;
};

void LoggingTestSuite::SetUp() {
  base::SetLogHandler(
      [](void* user_pointer, base::LogLevel ll, const char* msg) {
        auto* test_suite = reinterpret_cast<LoggingTestSuite*>(user_pointer);
        test_suite->last_msg_ = msg;
      },
      this);
}

void LoggingTestSuite::TearDown() {}

TEST_F(LoggingTestSuite, WriteFmtLogMsg) {
  LOG_INFO("hello {}", "world");
  EXPECT_STREQ(last_msg_.c_str(), "hello world");
}
}  // namespace