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

TEST(CommandLine, ClearTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe -a -b -c -d");
  line.Clear();

  EXPECT_EQ(line.parameter_count(), 0);
}

TEST(CommandLine, FindSwitchIndexTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe --foo=value");
  
  EXPECT_EQ(line.FindSwitchIndex(u8"foo"), 1);
  EXPECT_EQ(line.FindSwitchIndex(u8"bar"), -1);
}

TEST(CommandLine, FindSwitchValueTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe --foo=value");
  
  EXPECT_EQ(line.FindSwitchValue(u8"foo"), u8"value");
  EXPECT_EQ(line.FindSwitchValue(u8"bar"), u8"");
}

TEST(CommandLine, FindSwitchWithAliasTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe --foo=value");

  EXPECT_TRUE(line.FindSwitchWithAlias(u8"foo", u8"f"));
  EXPECT_FALSE(line.FindSwitchWithAlias(u8"bar", u8"b"));
}

TEST(CommandLine, ExtractSwitchValueTest) {
  base::CommandLine line;
  base::StringRefU8 item_contents = u8"--foo=value";

  EXPECT_EQ(base::CommandLine::ExtractSwitchValue(item_contents), u8"value");
}

TEST(CommandLine, FindPositionalArgumentsIndexTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe -a -b foo bar");

  EXPECT_EQ(line.FindPositionalArgumentsIndex(), 3);
}

TEST(CommandLine, IteratorTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe -a -b -c -d");
  auto iter = line.begin();

  EXPECT_EQ(*iter, u8"test.exe");
  ++iter;
  EXPECT_EQ(*iter, u8"-a");
}

TEST(CommandLine, ParseFromStringUTF8CharsTest) {
  base::CommandLine line;
  line.ParseFromString(u8"test.exe 往ガこ 往ガこ ん乇");

  EXPECT_EQ(line[0].length(), 8);
  EXPECT_EQ(line[1].length(), 9);
  EXPECT_EQ(line[2].length(), 9);
  EXPECT_EQ(line[3].length(), 6);
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