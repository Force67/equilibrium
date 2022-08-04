// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Do not edit.

#ifndef DISABLE_BASE_LOG
#include <base/logging.h>  // for the core logging api
#endif

#include <gtest/gtest.h>  // google test

int main(int argc, char** argv) {
#ifndef DISABLE_BASE_LOG
  base::SetLogHandler(
      [](void*, base::LogLevel log_level, const char* msg) { std::puts(msg); },
      nullptr);
#endif

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}