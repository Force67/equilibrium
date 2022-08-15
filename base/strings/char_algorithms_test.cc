// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/char_algorithms.h>
#if 0
namespace {
TEST(CharAlgos, CalculateStringLengthAscii) {
  constexpr char kTestStr[] = "HelloWorld";
  EXPECT_EQ(base::CalculateStringLength(kTestStr, sizeof(kTestStr)), 10);
}

TEST(CharAlgos, CalculateStringLengthWide) {
  constexpr wchar_t kTestStr[] = L"HelloWorld";
  EXPECT_EQ(base::CalculateStringLength(kTestStr, sizeof(kTestStr) / sizeof(wchar_t)), 10);
}

TEST(CharAlgos, CalculateAsciiStringLengthInvalidMaxSize) {
  constexpr char kTestStr[] = "HelloWorld";
  EXPECT_NE(base::CalculateStringLength(kTestStr, 4), 10);
}

TEST(CharAlgos, FindNullTerm) {
  constexpr char kTestStr[] = "HelloWorld";
  EXPECT_EQ(base::FindNullTerminator(kTestStr, sizeof(kTestStr)), 10);
}

TEST(CharAlgos, FailToFindNullTerm) {
  constexpr char kTestStr[] = {'a', 'b', 'c'};
  // not found
  EXPECT_EQ(base::FindNullTerminator(kTestStr, sizeof(kTestStr)), 0);

  constexpr char kTestStr2[] = "abcd";
  EXPECT_EQ(base::FindNullTerminator(kTestStr2, sizeof(kTestStr2)), 4);
}

}  // namespace
#endif