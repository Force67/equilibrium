// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/string_search.h>

namespace {
TEST(StringSearch, StringSearch) {
  constexpr char kTestStr[] = "HelloWorldBlahBlah";

  auto pos = base::StringSearch(kTestStr, sizeof(kTestStr), "World", 0, 5);
  EXPECT_EQ(pos, 5);

  pos = base::StringSearch(kTestStr, sizeof(kTestStr), "World", 10, 5);
  EXPECT_EQ(pos, base::kStringNotFoundPos);

  pos = base::StringSearch(kTestStr, sizeof(kTestStr), "BonJoviGreatestHits", 0, 19);
  EXPECT_EQ(pos, base::kStringNotFoundPos);
}

TEST(StringSearch, StringSearchWide) {
  constexpr wchar_t kTestStr[] = L"HelloWorldBlahBlah";
  auto pos = base::StringSearch(kTestStr, sizeof(kTestStr) * sizeof(wchar_t),
                                L"World", 0, 5);

  EXPECT_EQ(pos, 5);

  pos = base::StringSearch(kTestStr, sizeof(kTestStr) * sizeof(wchar_t),
                           L"BonJoviGreatestHits", 0, 19);
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