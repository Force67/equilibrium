// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "command_line.h"

#include <base/strings/char_algorithms.h>

namespace {
TEST(CommandLine, ParseFromString) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe -a -b -c -d");

  EXPECT_EQ(line[0].length(), 8);
  EXPECT_EQ(line[1].length(), 2);
  EXPECT_EQ(line[2].length(), 2);
  EXPECT_EQ(line[3].length(), 2);
  EXPECT_EQ(line[4].length(), 2);

  EXPECT_TRUE(line.FindSwitch(u8"a"));
  EXPECT_TRUE(line.FindSwitch(u8"b"));
  EXPECT_TRUE(line.FindSwitch(u8"c"));
  EXPECT_TRUE(line.FindSwitch(u8"d"));
}

TEST(CommandLine, ParseFromStringUTF8OnlyCharsTest) {
  //base::CommandLine line;
#if 0
  line.ParseFromString(u8"test.exe 往ガこ 往ガこ ん乇");
  
  EXPECT_EQ(line[0].length(), 8);
  EXPECT_EQ(line[1].length(), 3);
  EXPECT_EQ(line[2].length(), 3);
  EXPECT_EQ(line[3].length(), 2);
#endif
}
}  // namespace