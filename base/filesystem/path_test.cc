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

TEST(PathTests, AppendExtensionReportsWhetherItAppended) {
  // The POSIX definition used to run off the end of a bool function, so the
  // caller read whatever happened to be in the return register.
  base::Path plain("archive");
  EXPECT_TRUE(plain.AppendExtension("tar"));
  EXPECT_EQ(plain, base::Path("archive.tar"));

  // An extension that already carries the dot does not get a second one.
  base::Path dotted("archive");
  EXPECT_TRUE(dotted.AppendExtension(".tar"));
  EXPECT_EQ(dotted, base::Path("archive.tar"));

  // Without ensure_dot the extension is appended verbatim.
  base::Path undotted("archive");
  EXPECT_TRUE(undotted.AppendExtension("tar", /*ensure_dot=*/false));
  EXPECT_EQ(undotted, base::Path("archivetar"));

  // Nothing to append, and nothing appended.
  base::Path empty("archive");
  EXPECT_FALSE(empty.AppendExtension(""));
  EXPECT_EQ(empty, base::Path("archive"));
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

#if 0
TEST(PathTests, BaseName) {
  {
    base::Path a("c://abc//defg");
    auto base = a.BaseName();
    EXPECT_TRUE(base.c_str(), BASE_PATH_LITERAL("defg"));
  }

  {
    base::Path a("c://abc//defg.txt");
    auto base = a.BaseName();
    EXPECT_STREQ(base.c_str(), BASE_PATH_LITERAL("defg.txt"));
  }
}
#endif

}  // namespace