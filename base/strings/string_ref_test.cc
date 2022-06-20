// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/string_ref.h>

namespace {
TEST(StringRef, AssignfromString) {
  base::String str = "Hello World";

  base::StringRef ref(str);
  EXPECT_STREQ(str.c_str(), ref.c_str());
}

TEST(StringRef, AssignFromCStr) {
  constexpr char kTestStr[] = "Hello World";

  base::StringRef ref(kTestStr);
  EXPECT_EQ(ref.length(), 11);
  EXPECT_TRUE(ref.IsNullTerminated());
}

TEST(StringRef, AssignSubstring) {
  constexpr char kTestStr[] = "HelloWorld";

  base::StringRef ref(kTestStr, 4);
  EXPECT_EQ(ref.length(), 4);
  EXPECT_FALSE(ref.IsNullTerminated());
}

TEST(StringRef, At) {
  constexpr char kTestStr[] = "HelloWorld";

  base::StringRef ref(kTestStr);
  EXPECT_EQ(ref[2], 'l');
}
}  // namespace