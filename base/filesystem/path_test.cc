// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/filesystem/file.h>

namespace {

TEST(PathTests, ConstructEmpty) {
  base::Path p;
  EXPECT_TRUE(p.empty());
  EXPECT_FALSE(p);  // bool operator
}

TEST(PathTests, Append) {
#if defined(OS_WIN)
  base::Path p("C://l1");
  p.Append("l2");
  EXPECT_STREQ(p.c_str(), BASE_PATH_LITERAL("C:\\l1\\l2"));

  p /= "l3";
  EXPECT_STREQ(p.c_str(), BASE_PATH_LITERAL("C:\\l1\\l2\\l3"));

  base::Path p2 = p / "l4";
  EXPECT_STREQ(p2.c_str(), BASE_PATH_LITERAL("C:\\l1\\l2\\l3\\l4"));
#endif
}

TEST(PathTests, Compare) {
  base::Path a("c://abc");
  base::Path b("c://def");
  EXPECT_TRUE(a != b);
  EXPECT_FALSE(a == b);
}

TEST(PathTests, DirName) {
#if defined(OS_WIN)
  {
    base::Path a("c://abc//defg");
    auto dir_name = a.DirName();
    EXPECT_STREQ(dir_name.c_str(), BASE_PATH_LITERAL("c:\\abc"));
  }

  {
    base::Path a("c://abc//defg.txt");
    auto dir_name = a.DirName();
    EXPECT_STREQ(dir_name.c_str(), BASE_PATH_LITERAL("c:\\abc"));
  }
#endif
}

TEST(PathTests, BaseName) {
  {
    base::Path a("c://abc//defg");
    auto base = a.BaseName();
    EXPECT_STREQ(base.c_str(), BASE_PATH_LITERAL("defg"));
  }

  {
    base::Path a("c://abc//defg.txt");
    auto base = a.BaseName();
    EXPECT_STREQ(base.c_str(), BASE_PATH_LITERAL("defg.txt"));
  }
}

}  // namespace