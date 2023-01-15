// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "utf8_codepoint_iterator.h"

namespace {
using namespace base;
TEST(UTF8CodePointIteratorTest, TestAdvance) {
  char8_t test_string[] = u8"Hi 世界";
  base::StringRefU8 str(test_string);
  UTF8CodePointIterator iterator(str);

  EXPECT_EQ(iterator.array_pos(), 0);
  EXPECT_EQ(iterator.char_pos(), 0);
  EXPECT_EQ(iterator.current_character(), u8'H');
  EXPECT_FALSE(iterator.end());

  EXPECT_EQ(iterator.PeekCharacter(3), 0x4E16);

  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 1);
  EXPECT_EQ(iterator.char_pos(), 1);
  EXPECT_EQ(iterator.current_character(), 'i');
  EXPECT_FALSE(iterator.end());

  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 2);
  EXPECT_EQ(iterator.char_pos(), 2);
  EXPECT_EQ(iterator.current_character(), ' ');
  EXPECT_FALSE(iterator.end());

  EXPECT_EQ(iterator.PeekCharacter(2), 0x754C);

  // 世 -> \xe4\xb8\x96 (3 bytes)
  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 3);
  EXPECT_EQ(iterator.char_pos(), 3);
  EXPECT_EQ(iterator.current_character(), 0x4E16);
  EXPECT_FALSE(iterator.end());

  EXPECT_EQ(iterator.PeekCharacter(1), 0x754C);

  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 6);
  EXPECT_EQ(iterator.char_pos(), 4);
  EXPECT_EQ(iterator.current_character(), 0x754C);
  EXPECT_FALSE(iterator.end());

  EXPECT_EQ(iterator.array_pos(), 6 /*Remember, we didnt increment*/);
  EXPECT_EQ(iterator.char_pos(), 4); // we didnt advance
  EXPECT_FALSE(iterator.end());
}

TEST(UTF8CodePointIteratorTest, TestEnd) {
  char8_t test_string[] = u8"";
  base::StringRefU8 str(test_string);
  UTF8CodePointIterator iterator(str);
  EXPECT_TRUE(iterator.end());
}
}  // namespace