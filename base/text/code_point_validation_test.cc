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

TEST(DoIsStringUTF8Test, TestEmptyString) {
  const char8_t empty[] = u8"";
  EXPECT_TRUE(DoIsStringUTF8(empty, sizeof(empty)));
}

TEST(IsValidCodepoint, ValidCodePoints) {
  EXPECT_TRUE(IsValidCodepoint(0x0000));
  EXPECT_TRUE(IsValidCodepoint(0x0041));  // 'A'
  EXPECT_TRUE(IsValidCodepoint(0x4E16));  // '世'
  EXPECT_TRUE(IsValidCodepoint(0x10FFFF));
}

TEST(IsValidCodepoint, InvalidCodePoints) {
  EXPECT_FALSE(IsValidCodepoint(0xD800));
  EXPECT_FALSE(IsValidCodepoint(0xDFFF));
  EXPECT_FALSE(IsValidCodepoint(0x110000));
}

#if (OS_WIN)
TEST(IsMachineWordAligned, UnalignedPointers) {
  char array[sizeof(MachineWord) + 1];
  char* c;
  void* v;

  c = &array[1];
  EXPECT_FALSE(IsMachineWordAligned(c));
  v = (void*)c;
  EXPECT_FALSE(IsMachineWordAligned(v));
}
#endif

TEST(DoIsStringASCII, EmptyString) {
  const char* characters = "";
  mem_size length = 0;
  EXPECT_TRUE(DoIsStringASCII(characters, length));
}

TEST(DoIsStringASCII, ASCIIString) {
  const char characters[] = "Hello, world!";
  EXPECT_TRUE(DoIsStringASCII(characters, sizeof(characters) - 1));
}

TEST(DoIsStringASCII, NonASCIIString) {
  const char8_t characters[] = u8"Hello, 世界!";
  EXPECT_FALSE(DoIsStringASCII(characters, sizeof(characters) - 1));
}

TEST(DoIsStringASCII, MixedString) {
  const char8_t characters[] = u8"Hello, 世界! I am ASCII and Non-ASCII";
  EXPECT_FALSE(DoIsStringASCII(characters, sizeof(characters) - 1));
}
}  // namespace