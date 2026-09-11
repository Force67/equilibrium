// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/string_search.h>

namespace {
TEST(StringSearch, StringSearch) {
  constexpr char kTestStr[] = "HelloWorldBlahBlah";
  // Characters, not bytes, and without the terminator: |size| is the length of
  // the haystack that StringSearch is allowed to read.
  constexpr mem_size kLength = sizeof(kTestStr) / sizeof(char) - 1;

  auto pos = base::StringSearch(kTestStr, kLength, "World", 0, 5);
  EXPECT_EQ(pos, 5);

  pos = base::StringSearch(kTestStr, kLength, "World", 10, 5);
  EXPECT_EQ(pos, base::kStringNotFoundPos);

  pos = base::StringSearch(kTestStr, kLength, "BonJoviGreatestHits", 0, 19);
  EXPECT_EQ(pos, base::kStringNotFoundPos);
}

TEST(StringSearch, StringSearchWide) {
  constexpr wchar_t kTestStr[] = L"HelloWorldBlahBlah";
  constexpr mem_size kLength = sizeof(kTestStr) / sizeof(wchar_t) - 1;

  auto pos = base::StringSearch(kTestStr, kLength, L"World", 0, 5);
  EXPECT_EQ(pos, 5);

  pos = base::StringSearch(kTestStr, kLength, L"BonJoviGreatestHits", 0, 19);
  EXPECT_EQ(pos, base::kStringNotFoundPos);
}

TEST(StringSearch, FindLastOf) {
#if 0
  constexpr char kTestStr[] = "RepeatRepeatRepeat";
  auto pos = base::FindLastOf(kTestStr, sizeof(kTestStr), "Repeat", 0, 6);
  EXPECT_EQ(pos, 12);

  pos = base::FindLastOf(kTestStr, sizeof(kTestStr), "DontRepeat", 0, 10);
  EXPECT_EQ(pos, base::kStringNotFoundPos);
#endif
}

}  // namespace