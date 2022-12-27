// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "command_line.h"

#include <base/strings/char_algorithms.h>

namespace {
TEST(CommandLine, TestTokenize) {
  base::CommandLine line;
  line.Clear();

  line.InitFromWideCommandline(L"test.exe -a -b -c -d");
  #if 0
  EXPECT_EQ(line[0].length(), 8);
  EXPECT_EQ(line[1].length(), 2);
  EXPECT_EQ(line[2].length(), 2);
  EXPECT_EQ(line[3].length(), 2);
  EXPECT_EQ(line[4].length(), 2);
  #endif
}
}  // namespace