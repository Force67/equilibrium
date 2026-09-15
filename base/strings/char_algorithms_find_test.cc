// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/arch.h>
#include <base/strings/char_algorithms.h>

namespace {

TEST(CharAlgorithmsFind, FindCharLocatesFirstOccurrence) {
  const char* text = "a(b(c";
  EXPECT_EQ(base::FindChar(text, '('), text + 1);
  EXPECT_EQ(base::FindChar(text, 'a'), text);
  EXPECT_EQ(base::FindChar(text, 'c'), text + 4);
}

TEST(CharAlgorithmsFind, FindCharReturnsNullWhenAbsent) {
  EXPECT_EQ(base::FindChar("abc", 'z'), nullptr);
  EXPECT_EQ(base::FindChar("", 'z'), nullptr);
}

// strchr finds the terminator when asked for it; matching that keeps the
// "pointer to the end" idiom working.
TEST(CharAlgorithmsFind, FindCharFindsTheTerminator) {
  const char* text = "abc";
  EXPECT_EQ(base::FindChar(text, '\0'), text + 3);
  const char* empty = "";
  EXPECT_EQ(base::FindChar(empty, '\0'), empty);
}

TEST(CharAlgorithmsFind, FindLastCharLocatesLastOccurrence) {
  const char* text = "a(b(c";
  EXPECT_EQ(base::FindLastChar(text, '('), text + 3);
  EXPECT_EQ(base::FindLastChar(text, 'a'), text);
}

TEST(CharAlgorithmsFind, FindLastCharReturnsNullWhenAbsent) {
  EXPECT_EQ(base::FindLastChar("abc", 'z'), nullptr);
}

static_assert(*base::FindChar("a+b", '+') == '+');
static_assert(base::FindChar("abc", 'z') == nullptr);

}  // namespace
