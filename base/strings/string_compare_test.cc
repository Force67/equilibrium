// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/arch.h>
#include <base/strings/string_compare.h>

namespace {

// The whole point of the int return. These previously came back as mem_size,
// where a negative result wrapped to a huge positive one and every ordering
// test silently read backwards.
TEST(StringCompare, OrderingIsSignedAndNegativeWhenLeftSortsFirst) {
  EXPECT_LT(base::Strcmp("a", "b"), 0);
  EXPECT_GT(base::Strcmp("b", "a"), 0);
  EXPECT_LT(base::Strcmp("abc", "abd"), 0);
  EXPECT_GT(base::Strcmp("abd", "abc"), 0);
  // A prefix sorts before the longer string containing it.
  EXPECT_LT(base::Strcmp("ab", "abc"), 0);
  EXPECT_GT(base::Strcmp("abc", "ab"), 0);
}

TEST(StringCompare, EqualStringsCompareZero) {
  EXPECT_EQ(base::Strcmp("", ""), 0);
  EXPECT_EQ(base::Strcmp("abc", "abc"), 0);
  const char* same = "shared";
  EXPECT_EQ(base::Strcmp(same, same), 0);
}

TEST(StringCompare, EmptyStringSortsFirst) {
  EXPECT_LT(base::Strcmp("", "a"), 0);
  EXPECT_GT(base::Strcmp("a", ""), 0);
}

// Characters above 127 must compare as unsigned, or the answer flips on a
// target where plain char is signed (x86) versus unsigned (ARM).
TEST(StringCompare, HighBitCharactersCompareAsUnsigned) {
  const char high[] = {static_cast<char>(0x80), '\0'};
  const char low[] = {0x01, '\0'};
  EXPECT_GT(base::Strcmp(high, low), 0);
  EXPECT_LT(base::Strcmp(low, high), 0);

  const char ff[] = {static_cast<char>(0xFF), '\0'};
  EXPECT_GT(base::Strcmp(ff, high), 0);
}

TEST(StringCompare, StrncmpStopsAtTheLimit) {
  EXPECT_EQ(base::Strncmp("abcX", "abcY", 3), 0);
  EXPECT_LT(base::Strncmp("abcX", "abcY", 4), 0);
  EXPECT_EQ(base::Strncmp("abc", "abc", 100), 0);
}

// The old implementation read two uninitialized locals on this path.
TEST(StringCompare, StrncmpWithZeroLimitComparesEqual) {
  EXPECT_EQ(base::Strncmp("anything", "different", 0), 0);
  EXPECT_EQ(base::Strncmp("", "x", 0), 0);
}

TEST(StringCompare, StrncmpStopsAtTheTerminator) {
  // Equal and both ended before the limit: no read past the terminator.
  EXPECT_EQ(base::Strncmp("ab", "ab", 64), 0);
  EXPECT_GT(base::Strncmp("abc", "ab", 64), 0);
  EXPECT_LT(base::Strncmp("ab", "abc", 64), 0);
}

TEST(StringCompare, StrEqual) {
  EXPECT_TRUE(base::StrEqual("abc", "abc"));
  EXPECT_TRUE(base::StrEqual("", ""));
  EXPECT_FALSE(base::StrEqual("abc", "abd"));
  EXPECT_FALSE(base::StrEqual("ab", "abc"));
}

TEST(StringCompare, WideCharacters) {
  EXPECT_EQ(base::Strcmp(u"abc", u"abc"), 0);
  EXPECT_LT(base::Strcmp(u"abc", u"abd"), 0);
  EXPECT_EQ(base::Strcmp(U"abc", U"abc"), 0);
  EXPECT_GT(base::Strcmp(U"abd", U"abc"), 0);
}

// Usable in a constant expression, which the C function is not.
static_assert(base::Strcmp("abc", "abc") == 0);
static_assert(base::Strcmp("a", "b") < 0);
static_assert(base::Strcmp("b", "a") > 0);
static_assert(base::Strncmp("abcX", "abcY", 3) == 0);
static_assert(base::StrEqual("hello", "hello"));

}  // namespace
