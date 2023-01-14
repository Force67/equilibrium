// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "code_point_validation.h"

namespace {
using namespace base;
TEST(IsValidUTF8CharacterTest, ValidCodePoints) {
  EXPECT_TRUE(IsValidUTF8Character(0x0000));
  EXPECT_TRUE(IsValidUTF8Character(0xD7FF));
  EXPECT_TRUE(IsValidUTF8Character(0xE000));
}

TEST(IsValidUTF8CharacterTest, InvalidCodePoints) {
  EXPECT_FALSE(IsValidUTF8Character(0xD800));
  EXPECT_FALSE(IsValidUTF8Character(0xFDD0));
  EXPECT_FALSE(IsValidUTF8Character(0xFFFE));
  EXPECT_FALSE(IsValidUTF8Character(0xFFFF));
  EXPECT_FALSE(IsValidUTF8Character(0xFDEF));
}

TEST(DoIsStringUTF8Test, ValidString) {
  const char str[] = "Hello, world!";
  EXPECT_TRUE(DoIsStringUTF8(str, sizeof(str)));
}

TEST(DoIsStringUTF8Test, InvalidString) {
  const char str[] = "\xF0\x28\x8C\x28";  // invalid UTF-8 sequence
  EXPECT_FALSE(DoIsStringUTF8(str, sizeof(str)));
}

TEST(DoIsStringUTF8Test, TestEmoji) {
  const char8_t emoji[] = u8"😂";
  EXPECT_TRUE(DoIsStringUTF8(emoji, sizeof(emoji)));
}

TEST(DoIsStringUTF8Test, TestForeignLanguage) {
  const char8_t foreign[] = u8"你好";
  EXPECT_TRUE(DoIsStringUTF8(foreign, sizeof(foreign)));
}
}  // namespace