// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "utf8_codepoint_iterator.h"

namespace {
using namespace base;
TEST(UTF8CodePointIteratorTest, TestAdvance) {
  char8_t test_string[] = u8"Hello, 世界";
  base::StringRefU8 str(test_string);
  UTF8CodePointIterator iterator(str);

  EXPECT_EQ(iterator.array_pos(), 0);
  EXPECT_EQ(iterator.char_pos(), 0);
  EXPECT_EQ(iterator.current_character(), u8'H');
  EXPECT_FALSE(iterator.end());

  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 1);
  EXPECT_EQ(iterator.char_pos(), 1);
  EXPECT_EQ(iterator.current_character(), 'e');
  EXPECT_FALSE(iterator.end());

  EXPECT_TRUE(iterator.Advance());
  EXPECT_EQ(iterator.array_pos(), 2);
  EXPECT_EQ(iterator.char_pos(), 2);
  EXPECT_EQ(iterator.current_character(), 'l');
  EXPECT_FALSE(iterator.end());

  // Advance to the end of the string
  while (iterator.Advance())
    ;
  EXPECT_EQ(iterator.array_pos(), sizeof(test_string) - 1);
  EXPECT_EQ(iterator.char_pos(), 9);
  EXPECT_TRUE(iterator.end());
}

TEST(UTF8CodePointIteratorTest, TestEnd) {
  char8_t test_string[] = u8"";
  base::StringRefU8 str(test_string);
  UTF8CodePointIterator iterator(str);
  EXPECT_TRUE(iterator.end());
}
}  // namespace