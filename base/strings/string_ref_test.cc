// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/strings/string_ref.h>

namespace {
using namespace base;

TEST(BasicStringRefTest, Construction) {
  std::string test_str = "Hello, world!";
  StringRef str_ref(test_str);
  EXPECT_TRUE(str_ref.IsNullTerminated());
  EXPECT_EQ(str_ref.length(), test_str.length());
  EXPECT_STREQ(str_ref.data(), test_str.data());
}

TEST(BasicStringRefTest, ConstructNullRef) {
  StringRef str_ref = StringRef::null_ref();
  EXPECT_TRUE(str_ref.IsNullTerminated());
  EXPECT_EQ(str_ref.length(), 0);
  EXPECT_STREQ(str_ref.data(), "");
}

TEST(BasicStringRefTest, ConstexprConstruction) {
  std::string test_str = "Hello, world!";

  // TODO: fix constexpr construction
  constexpr StringRef str_ref("Hello, world!");

  EXPECT_TRUE(str_ref.IsNullTerminated());
  EXPECT_EQ(str_ref.length(), test_str.length());
  EXPECT_STREQ(str_ref.data(), test_str.data());
}

TEST(BasicStringRefTest, NullTermination) {
  const char test_data[] = {'H', 'e', 'l', 'l', 'o'};
  StringRef str_ref(test_data, sizeof(test_data));

  EXPECT_EQ(str_ref.length(), sizeof(test_data));
  EXPECT_FALSE(str_ref.IsNullTerminated());
}

TEST(BasicStringRefTest, OperatorBrackets) {
  std::string test_str = "Hello, world!";
  StringRef str_ref(test_str);

  for (size_t i = 0; i < test_str.length(); ++i) {
    EXPECT_EQ(str_ref[i], test_str[i]);
  }
}

TEST(BasicStringRefTest, ComparisonOperators) {
  StringRef str_ref1("Hello, world!");
  StringRef str_ref2("Hello, world!");
  StringRef str_ref3("Goodbye, world!");

  EXPECT_TRUE(str_ref1 == str_ref2);
  EXPECT_FALSE(str_ref1 != str_ref2);
  EXPECT_FALSE(str_ref1 == str_ref3);
  EXPECT_TRUE(str_ref1 != str_ref3);

  EXPECT_TRUE(str_ref1 <= str_ref2);
  EXPECT_TRUE(str_ref1 >= str_ref2);

  // Lexicographic: 'G' sorts before 'H'.
  EXPECT_TRUE(str_ref3 < str_ref1);
  EXPECT_TRUE(str_ref3 <= str_ref1);
  EXPECT_TRUE(str_ref1 > str_ref3);
  EXPECT_TRUE(str_ref1 >= str_ref3);
}

TEST(BasicStringRefTest, Find) {
  StringRef str_ref("Hello, world!");

  EXPECT_EQ(str_ref.find("world", 0, 5), 7u);
  EXPECT_EQ(str_ref.find("not_found", 0, 9), StringRef::npos);
}

TEST(BasicStringRefTest, StartsWith) {
  StringRef ref("data/meshes/rock.nif");

  EXPECT_TRUE(ref.starts_with('d'));
  EXPECT_TRUE(ref.starts_with("data/"));
  EXPECT_TRUE(ref.starts_with(StringRef("data")));
  EXPECT_FALSE(ref.starts_with("meshes"));
}

TEST(BasicStringRefTest, EndsWith) {
  StringRef ref("data/meshes/rock.nif");

  EXPECT_TRUE(ref.ends_with('f'));
  EXPECT_TRUE(ref.ends_with(".nif"));
  EXPECT_TRUE(ref.ends_with(StringRef("rock.nif")));
  EXPECT_FALSE(ref.ends_with(".dds"));
}

TEST(BasicStringRefTest, AffixLongerThanTheRefIsNotAMatch) {
  StringRef ref("ab");

  EXPECT_FALSE(ref.starts_with("abcd"));
  EXPECT_FALSE(ref.ends_with("abcd"));
}

TEST(BasicStringRefTest, ReverseFind) {
  StringRef ref("a/b/c");

  EXPECT_EQ(ref.rfind('/'), 3u);
  EXPECT_EQ(ref.rfind('/', 2), 1u);
  EXPECT_EQ(ref.rfind("b/"), 2u);
  EXPECT_EQ(ref.rfind("zz"), StringRef::npos);
}

TEST(BasicStringRefTest, FrontAndBack) {
  StringRef ref("abc");

  EXPECT_EQ(ref.front(), 'a');
  EXPECT_EQ(ref.back(), 'c');
}

TEST(BasicStringRefTest, RemoveSuffixAndPrefix) {
  StringRef ref("meshes/rock.nif");

  ref.remove_suffix(4);
  EXPECT_EQ(ref.size(), 11u);
  EXPECT_TRUE(ref.ends_with("rock"));

  ref.remove_prefix(7);
  EXPECT_EQ(ref.size(), 4u);
  EXPECT_TRUE(ref.starts_with("rock"));
}

TEST(BasicStringRefTest, PositionalCompare) {
  StringRef ref("data/meshes");

  EXPECT_EQ(ref.compare(0, 4, StringRef("data")), 0);
  EXPECT_NE(ref.compare(0, 4, StringRef("meta")), 0);
  EXPECT_LT(ref.compare(0, 4, StringRef("datax")), 0);
}

TEST(BasicStringRefTest, FindAnotherRef) {
  StringRef ref("Hello, world!");

  EXPECT_EQ(ref.find(StringRef("world")), 7u);
  EXPECT_EQ(ref.find(StringRef("moon")), StringRef::npos);
}

// Regression: substr used to return an owning base::String, so binding the
// result to a StringRef left it pointing into a destroyed temporary.
TEST(BasicStringRefTest, SubstrReturnsAViewOfTheSameBuffer) {
  StringRef ref("meshes/rock.nif");

  StringRef stem = ref.substr(7);

  EXPECT_EQ(stem.size(), 8u);
  EXPECT_EQ(stem.data(), ref.data() + 7);
  EXPECT_TRUE(stem == StringRef("rock.nif"));
}

TEST(BasicStringRefTest, SubstrSurvivesNestedCalls) {
  StringRef ref("a/b/c");

  StringRef tail = ref.substr(2).substr(2);

  EXPECT_TRUE(tail == StringRef("c"));
}

TEST(BasicStringRefTest, SubstrClampsTheCount) {
  StringRef ref("abc");

  EXPECT_EQ(ref.substr(1, 99).size(), 2u);
  EXPECT_EQ(ref.substr(3).size(), 0u);
}

TEST(BasicStringRefTest, ToStringCopiesOut) {
  StringRef ref("abcdef");

  const base::String owned = ref.substr(2, 3).to_string();

  EXPECT_EQ(owned, "cde");
}

TEST(BasicStringRefTest, Contains) {
  StringRef ref("Hello, world!");

  EXPECT_TRUE(ref.contains(','));
  EXPECT_TRUE(ref.contains("world"));
  EXPECT_FALSE(ref.contains("moon"));
}
}  // namespace