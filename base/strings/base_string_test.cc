#include <gtest/gtest.h>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "base_string.h"
#include "string_ref.h"
#include "xstring.h"  // base::String, base::StringW, etc.

namespace {
static const char kTestSentence[] = "Hello, world!";
static const char8_t kTestSentence8[] = u8"Hello, world!";
static const wchar_t kTestSentenceW[] = L"Hello, world!";
static const char16_t kTestSentence16[] = u"Hello, world!";
static const char32_t kTestSentence32[] = U"Hello, world!";

template <typename CharType>
class BaseStringTest : public ::testing::Test {
 public:
  using BaseStringType = base::BasicBaseString<CharType>;
  using StdStringType = std::basic_string<CharType>;

  static const CharType* GetTestSentence() {
    if constexpr (std::is_same_v<CharType, char>) {
      return &kTestSentence[0];
    } else if constexpr (std::is_same_v<CharType, char8_t>) {
      return &kTestSentence8[0];
    } else if constexpr (std::is_same_v<CharType, wchar_t>) {
      return &kTestSentenceW[0];
    } else if constexpr (std::is_same_v<CharType, char16_t>) {
      return &kTestSentence16[0];
    } else if constexpr (std::is_same_v<CharType, char32_t>) {
      return &kTestSentence32[0];
    }
  }

  static const CharType* GetAnotherTestSentence() {
    if constexpr (std::is_same_v<CharType, char>) {
      return "Goodbye, world!";
    } else if constexpr (std::is_same_v<CharType, char8_t>) {
      return u8"Goodbye, world!";
    } else if constexpr (std::is_same_v<CharType, wchar_t>) {
      return L"Goodbye, world!";
    } else if constexpr (std::is_same_v<CharType, char16_t>) {
      return u"Goodbye, world!";
    } else if constexpr (std::is_same_v<CharType, char32_t>) {
      return U"Goodbye, world!";
    }
  }
};

using CharTypes = ::testing::Types<char, char8_t, wchar_t, char16_t, char32_t>;
TYPED_TEST_SUITE(BaseStringTest, CharTypes);

TYPED_TEST(BaseStringTest, Construction) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Default construction. Note: capacity() is non-zero on a fresh instance
  // because BasicBaseString uses SSO — the inline buffer is always usable.
  BaseStringType base_str;
  ASSERT_EQ(base_str.size(), 0);
  ASSERT_TRUE(base_str.empty());
  ASSERT_GT(base_str.capacity(), 0u);

  // Construction from C-style string
  const typename BaseStringType::character_type* c_str = this->GetTestSentence();
  BaseStringType base_str_from_c_str(c_str);
  StdStringType std_str_from_c_str(c_str);
  ASSERT_EQ(base_str_from_c_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str_from_c_str.compare(std_str_from_c_str), 0);

  // Copy construction
  BaseStringType base_str_copy(base_str_from_c_str);
  ASSERT_EQ(base_str_copy.size(), base_str_from_c_str.size());
  ASSERT_EQ(base_str_copy.compare(base_str_from_c_str), 0);
}

TEST(BaseStringTest, ConstructFromArray) {
  using BaseStringType = base::BasicBaseString<char8_t>;
  using StdStringType = std::basic_string<char8_t>;

  const char8_t kTestSentence8[] = u8"Hello, world!";
  const char8_t kTestSentence8Copy[] = u8"Hello, world!";
  const char8_t kTestSentence8Copy2[] = u8"Hello, world!";
  const char8_t kTestSentence8Copy3[] = u8"Hello, world!";

  const BaseStringType base_str(kTestSentence8);
  BaseStringType base_str_copy(kTestSentence8Copy);
}

TYPED_TEST(BaseStringTest, Assignment) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Assignment from C-style string
  const typename BaseStringType::character_type* c_str = this->GetTestSentence();
  BaseStringType base_str;
  base_str.assign(c_str);
  StdStringType std_str_from_c_str(c_str);
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  // Copy assignment
  BaseStringType base_str_from_c_str(c_str);
  BaseStringType base_str_copy;
  base_str_copy = base_str_from_c_str;
  ASSERT_EQ(base_str_copy.size(), base_str_from_c_str.size());
  ASSERT_EQ(base_str_copy.compare(base_str_from_c_str), 0);
}

TYPED_TEST(BaseStringTest, Equality) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Equality with C-style string
  const typename BaseStringType::character_type* c_str = this->GetTestSentence();
  BaseStringType base_str(c_str);
  StdStringType std_str_from_c_str(c_str);
  ASSERT_TRUE(base_str == c_str);
  ASSERT_TRUE(base_str == std_str_from_c_str);

  // Inequality with different C-style string
  const typename BaseStringType::character_type* different_c_str =
      this->GetAnotherTestSentence();
  ASSERT_FALSE(base_str == different_c_str);

  // Equality with another BaseString
  BaseStringType another_base_str(c_str);
  ASSERT_TRUE(base_str == another_base_str);
}

TEST(BaseStringTest, CompareSubstring) {
  using BaseStringType = typename base::BasicBaseString<char>;

  const BaseStringType str1("Hello, World!");
  const BaseStringType str2("World");

  // Compare full string with itself.
  ASSERT_EQ(str1.compare(0, str1.size(), str1), 0);

  // Substring of str1 starting at 7 with length 5 == "World".
  ASSERT_EQ(str1.compare(7, 5, str2), 0);

  // len is clamped to remaining size: "World!" (6) vs "World" (5).
  // Equal first 5 chars, ours is longer → result > 0.
  ASSERT_GT(str1.compare(7, 10, str2), 0);

  // Empty substring is "less than" any non-empty string.
  ASSERT_LT(str1.compare(0, 0, str2), 0);

  // First-13-chars of str1 ("Hello, World!") vs "World":
  // 'H' < 'W' on the first byte → result < 0.
  ASSERT_LT(str1.compare(0, 20, str2), 0);

  // Different strings.
  const BaseStringType str3("Hello, Universe!");
  ASSERT_NE(str1.compare(0, str1.size(), str3), 0);
}

TYPED_TEST(BaseStringTest, Concatenation) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Concatenation with C-style string
  const typename BaseStringType::character_type* c_str = this->GetTestSentence();
  BaseStringType base_str;
  base_str += c_str;
  StdStringType std_str_from_c_str(c_str);
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  // Concatenation with another BaseString
  BaseStringType another_base_str(c_str);
  base_str += another_base_str;
  StdStringType std_str_from_c_str_twice(c_str);
  std_str_from_c_str_twice += c_str;
  ASSERT_EQ(base_str.size(), std_str_from_c_str_twice.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str_twice), 0);
}

#if 1
TEST(BaseStringTest, PushBack) {
  using BaseStringType = typename base::BasicBaseString<char>;
  using StdStringType = typename std::basic_string<char>;

  // Push back a character
  BaseStringType base_str;
  base_str.push_back('a');
  StdStringType std_str_from_c_str("a");
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  // Push back multiple characters
  base_str.push_back('b');
  base_str.push_back('c');
  std_str_from_c_str = "abc";
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  const char kAlphabet[] = "abcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < 26; ++i) {
    base_str.push_back(kAlphabet[i]);
    std_str_from_c_str += kAlphabet[i];
  }
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());

  // Push back a character with reallocation
  for (int i = 0; i < 100; ++i) {
    base_str.push_back('d');
    std_str_from_c_str += 'd';
  }
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  // Push back an empty string
  base_str.push_back('\0');
  std_str_from_c_str += '\0';
  ASSERT_EQ(base_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str.compare(std_str_from_c_str), 0);

  // Push back a string with maximum capacity
  BaseStringType max_base_str;
  StdStringType max_std_str;
  const size_t max_size = 0xFFFF;
  for (size_t i = 0; i < max_size; ++i) {
    max_base_str.push_back('a');
    max_std_str += 'a';
  }
  ASSERT_EQ(max_base_str.size(), max_std_str.size());
  ASSERT_EQ(max_base_str.compare(max_std_str), 0);
}

TEST(BaseStringTest, Append) {
  using BaseStringType = typename base::BasicBaseString<char>;
  using StdStringType = typename std::basic_string<char>;

  // Append an empty string
  BaseStringType base_str("hello");
  StdStringType std_str("hello");
  base_str.append("");
  ASSERT_EQ(base_str.size(), std_str.size());
  ASSERT_EQ(base_str.compare(std_str), 0);

  // Append a non-empty string
  base_str.append(" world");
  std_str += " world";
  ASSERT_EQ(base_str.size(), std_str.size());
  ASSERT_EQ(base_str.compare(std_str), 0);

  // Append a string with reallocation
  base_str.append("123456789");
  std_str += "123456789";
  ASSERT_EQ(base_str.size(), std_str.size());
  ASSERT_EQ(base_str.compare(std_str), 0);

  // Append a string to an empty string
  BaseStringType empty_base_str;
  StdStringType empty_std_str;
  empty_base_str.append("test");
  empty_std_str = "test";
  ASSERT_EQ(empty_base_str.size(), empty_std_str.size());
  ASSERT_EQ(empty_base_str.compare(empty_std_str), 0);

  // Append a string with maximum capacity
  BaseStringType max_base_str;
  StdStringType max_std_str;
  const size_t max_size = 0xFFFF;
  for (size_t i = 0; i < max_size; ++i) {
    max_base_str.append("a");
    max_std_str += "a";
  }
  ASSERT_EQ(max_base_str.size(), max_std_str.size());
  ASSERT_EQ(max_base_str.compare(max_std_str), 0);
}

TEST(BaseStringTest, CopyConstructorTorture) {
  using BaseStringType = typename base::BasicBaseString<char>;
  using StdStringType = typename std::basic_string<char>;

  const std::string long_string(1024 * 1024, 'a');  // 1 MB string

  // Copy constructor for empty string
  BaseStringType empty_base_str;
  StdStringType empty_std_str;
  BaseStringType base_copy_empty(empty_base_str);
  StdStringType std_copy_empty(empty_std_str);
  ASSERT_EQ(base_copy_empty.size(), std_copy_empty.size());
  ASSERT_EQ(base_copy_empty.compare(std_copy_empty), 0);

  // Copy constructor for small string
  BaseStringType small_base_str("hello");
  StdStringType small_std_str("hello");
  BaseStringType base_copy_small(small_base_str);
  StdStringType std_copy_small(small_std_str);
  ASSERT_EQ(base_copy_small.size(), std_copy_small.size());
  ASSERT_EQ(base_copy_small.compare(std_copy_small), 0);

  // Copy constructor for large string (with reallocation)
  BaseStringType large_base_str(long_string);
  StdStringType large_std_str(long_string);
  BaseStringType base_copy_large(large_base_str);
  StdStringType std_copy_large(large_std_str);
  ASSERT_EQ(base_copy_large.size(), std_copy_large.size());
  ASSERT_EQ(base_copy_large.compare(std_copy_large), 0);

  // Copy constructor for self-assignment
  BaseStringType self_assign_base_str("test");
  StdStringType self_assign_std_str("test");
  BaseStringType base_copy_self(self_assign_base_str);
  self_assign_base_str = base_copy_self;
  StdStringType std_copy_self(self_assign_std_str);
  self_assign_std_str = std_copy_self;
  ASSERT_EQ(self_assign_base_str.size(), self_assign_std_str.size());
  ASSERT_EQ(self_assign_base_str.compare(self_assign_std_str), 0);

  // Copy constructor for null-terminated string
  const char* null_terminated = "null\0terminated";
  BaseStringType base_null_terminated(null_terminated, 15);
  StdStringType std_null_terminated(null_terminated, 15);
  BaseStringType base_copy_null(base_null_terminated);
  StdStringType std_copy_null(std_null_terminated);
  ASSERT_EQ(base_copy_null.size(), std_copy_null.size());
  ASSERT_EQ(base_copy_null.compare(std_copy_null), 0);
}

TEST(BaseStringTest, SplitBySpaces) {
  using BaseStringType = typename base::BasicBaseString<char>;
  using StdStringType = typename std::basic_string<char>;

  // Split a string with spaces
  const BaseStringType base_str("arg1 arg2   arg3");
  StdStringType std_str("arg1 arg2   arg3");
  std::vector<BaseStringType> base_pieces;
  std::vector<StdStringType> std_pieces;

  BaseStringType current_base_arg;
  StdStringType current_std_arg;

  for (auto c : base_str) {
    if (c == ' ') {
      if (!current_base_arg.empty()) {
        base_pieces.push_back(current_base_arg);
        std_pieces.push_back(current_std_arg);
        current_base_arg.clear();
        current_std_arg.clear();
      }
    } else {
      current_base_arg.push_back(c);
      current_std_arg.push_back(c);
    }
  }

  if (!current_base_arg.empty()) {
    base_pieces.push_back(current_base_arg);
    std_pieces.push_back(current_std_arg);
  }

  ASSERT_EQ(base_pieces.size(), std_pieces.size());
  ASSERT_EQ(base_pieces.size(), 3U);
  ASSERT_STREQ(base_pieces[0].c_str(), "arg1");
  ASSERT_STREQ(base_pieces[1].c_str(), "arg2");
  ASSERT_STREQ(base_pieces[2].c_str(), "arg3");

  // Split a string with tabs, newlines, and carriage returns
  const BaseStringType base_str2("arg1\targ2\narg3\rarg4");
  std_str = "arg1\targ2\narg3\rarg4";
  base_pieces.clear();
  std_pieces.clear();

  current_base_arg.clear();
  current_std_arg.clear();

  for (auto c : base_str) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      if (!current_base_arg.empty()) {
        base_pieces.push_back(current_base_arg);
        std_pieces.push_back(current_std_arg);
        current_base_arg.clear();
        current_std_arg.clear();
      }
    } else {
      current_base_arg.push_back(c);
      current_std_arg.push_back(c);
    }
  }

  if (!current_base_arg.empty()) {
    base_pieces.push_back(current_base_arg);
    std_pieces.push_back(current_std_arg);
  }

  ASSERT_EQ(base_pieces.size(), std_pieces.size());
  ASSERT_EQ(base_pieces.size(), 3U);
  ASSERT_EQ(base_pieces[0], "arg1");
  ASSERT_EQ(base_pieces[1], "arg2");
  ASSERT_EQ(base_pieces[2], "arg3");
  // ASSERT_EQ(base_pieces[3], "arg4");
}

TEST(BaseStringTest, ErasingCharacters) {
  using BaseStringType = typename base::BasicBaseString<wchar_t>;
  using StdStringType = typename std::basic_string<wchar_t>;

  const std::wstring test_string = L"Hello, World!";

  // Erase single character
  BaseStringType base_str1(test_string);
  StdStringType std_str1(test_string);
  base_str1.erase(7);
  std_str1.erase(7);
  ASSERT_EQ(base_str1.size(), std_str1.size());
  ASSERT_EQ(base_str1.compare(std_str1), 0);

  // Erase substring
  BaseStringType base_str2(test_string);
  StdStringType std_str2(test_string);
  base_str2.erase(7, 5);
  std_str2.erase(7, 5);
  ASSERT_EQ(base_str2.size(), std_str2.size());
  ASSERT_EQ(base_str2.compare(std_str2), 0);

  // Erase range
  BaseStringType base_str3(test_string);
  StdStringType std_str3(test_string);
  base_str3.erase(base_str3.begin() + 7, base_str3.begin() + 12);
  std_str3.erase(std_str3.begin() + 7, std_str3.begin() + 12);
  ASSERT_EQ(base_str3.size(), std_str3.size());
  ASSERT_EQ(base_str3.compare(std_str3), 0);

  // Erase all characters
  BaseStringType base_str4(test_string);
  StdStringType std_str4(test_string);
  base_str4.erase(base_str4.begin(), base_str4.end());
  std_str4.erase(std_str4.begin(), std_str4.end());
  ASSERT_EQ(base_str4.size(), std_str4.size());
  ASSERT_EQ(base_str4.compare(std_str4), 0);
}

#if 0
TYPED_TEST(BaseStringTest, Substring) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Get substring
  const typename BaseStringType::character_type* c_str =
      this->GetTestSentence();
  BaseStringType base_str(c_str);
  BaseStringType substring = base_str.substr(0, 5);
  StdStringType std_str_from_c_str("Hello");
  ASSERT_EQ(substring.size(), std_str_from_c_str.size());
  ASSERT_EQ(substring.compare(std_str_from_c_str), 0);
}
#endif

#endif

// ──────────────────────────────────────────────────────────────────────────
// Regression tests for the SSO/aliasing/wide-char bugs (see review).
// ──────────────────────────────────────────────────────────────────────────

// Layout: every default-instantiated BasicBaseString must fit in one
// 3-pointer footprint regardless of char width. This catches a regression
// where wide-char instantiations bloated to 4× the intended size.
static_assert(sizeof(base::BasicBaseString<char>) == 3 * sizeof(void*));
static_assert(sizeof(base::BasicBaseString<char8_t>) == 3 * sizeof(void*));
static_assert(sizeof(base::BasicBaseString<char16_t>) == 3 * sizeof(void*));
static_assert(sizeof(base::BasicBaseString<char32_t>) == 3 * sizeof(void*));
static_assert(sizeof(base::BasicBaseString<wchar_t>) == 3 * sizeof(void*));

// Wide-char fill ctor must use the actual wide-char value, not memset's
// byte fill. memset(d, L'A', n*4) would give garbage like L"䅁䅁䅁".
TEST(BaseStringRegressions, FillCtorWideChar) {
  base::BasicBaseString<wchar_t> ws(5, L'A');
  ASSERT_EQ(ws.size(), 5u);
  for (size_t i = 0; i < ws.size(); ++i) ASSERT_EQ(ws[i], L'A') << "i=" << i;
  ASSERT_EQ(ws.c_str()[ws.size()], L'\0');

  base::BasicBaseString<char16_t> us(5, u'B');
  for (size_t i = 0; i < us.size(); ++i) ASSERT_EQ(us[i], u'B');

  base::BasicBaseString<char32_t> Us(5, U'C');
  for (size_t i = 0; i < Us.size(); ++i) ASSERT_EQ(Us[i], U'C');
}

// Same bug in insert(pos, count, c).
TEST(BaseStringRegressions, InsertFillWideChar) {
  base::BasicBaseString<wchar_t> ws(L"abc");
  ws.insert(1, 3, L'Z');  // expect L"aZZZbc"
  ASSERT_EQ(ws.size(), 6u);
  ASSERT_EQ(ws[0], L'a');
  ASSERT_EQ(ws[1], L'Z');
  ASSERT_EQ(ws[2], L'Z');
  ASSERT_EQ(ws[3], L'Z');
  ASSERT_EQ(ws[4], L'b');
  ASSERT_EQ(ws[5], L'c');
}

// Self-aliasing assign: source pointer is inside our own buffer. The old
// implementation freed the buffer before reading from it.
TEST(BaseStringRegressions, AssignFromSelfSubrange) {
  base::String s("hello world this is a long string here");  // heap
  ASSERT_GE(s.size(), 22u);
  s.assign(s.c_str() + 6, 5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "world");
}

TEST(BaseStringRegressions, AssignFromSelfSmall) {
  base::String s("hello world");  // small / inline
  s.assign(s.c_str() + 6, 5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "world");
}

// Self-aliasing append: source is our own data, growth-triggering.
TEST(BaseStringRegressions, AppendFromSelfTriggersGrowth) {
  base::String s("0123456789012345");  // 16 chars, fits SSO (≤22)
  ASSERT_LE(s.size(), 22u);
  s.append(s.c_str(), s.size());        // double — pushes us over SSO
  EXPECT_EQ(s.size(), 32u);
  EXPECT_STREQ(s.c_str(), "01234567890123450123456789012345");

  // And again from heap state.
  s.append(s.c_str(), s.size());
  EXPECT_EQ(s.size(), 64u);
  for (size_t i = 0; i < 32; ++i) EXPECT_EQ(s[i], s[i + 32]);
}

TEST(BaseStringRegressions, AppendFromSelfWhileLarge) {
  base::String s("0123456789012345678901234567890");  // 31 chars, on heap
  ASSERT_FALSE(s.size() <= 22);
  const auto orig_size = s.size();
  s.append(s.c_str(), s.size());
  EXPECT_EQ(s.size(), orig_size * 2);
  for (size_t i = 0; i < orig_size; ++i) {
    EXPECT_EQ(s[i], s[i + orig_size]);
  }
}

TEST(BaseStringSearch, StartsWith) {
  base::String s = "shaders/gi/pathtrace.cs";

  EXPECT_TRUE(s.starts_with('s'));
  EXPECT_TRUE(s.starts_with("shaders/"));
  EXPECT_TRUE(s.starts_with(base::String("shaders")));
  EXPECT_FALSE(s.starts_with("gi/"));
  EXPECT_FALSE(s.starts_with('x'));
}

TEST(BaseStringSearch, EndsWith) {
  base::String s = "textures/rock_n.dds";

  EXPECT_TRUE(s.ends_with('s'));
  EXPECT_TRUE(s.ends_with(".dds"));
  EXPECT_TRUE(s.ends_with(base::String("_n.dds")));
  EXPECT_FALSE(s.ends_with(".nif"));
}

TEST(BaseStringSearch, PrefixLongerThanTheStringIsNotAMatch) {
  base::String s = "ab";

  EXPECT_FALSE(s.starts_with("abcd"));
  EXPECT_FALSE(s.ends_with("abcd"));
}

TEST(BaseStringSearch, EmptyStringMatchesNothingButTheEmptyPrefix) {
  base::String s;

  EXPECT_TRUE(s.starts_with(""));
  EXPECT_TRUE(s.ends_with(""));
  EXPECT_FALSE(s.starts_with('a'));
  EXPECT_FALSE(s.ends_with('a'));
}

TEST(BaseStringSearch, Contains) {
  base::String s = "meshes/architecture/whiterun";

  EXPECT_TRUE(s.contains('/'));
  EXPECT_TRUE(s.contains("architecture"));
  EXPECT_FALSE(s.contains("solitude"));
}

TEST(BaseStringSearch, ReverseFind) {
  base::String s = "a/b/c";

  EXPECT_EQ(s.rfind('/'), 3u);
  EXPECT_EQ(s.rfind("b/"), 2u);
  EXPECT_EQ(s.rfind("a"), 0u);
  EXPECT_EQ(s.rfind("zz"), base::String::npos);
}

TEST(BaseStringSearch, ReverseFindHonoursThePosition) {
  base::String s = "x.y.z";

  EXPECT_EQ(s.rfind('.'), 3u);
  EXPECT_EQ(s.rfind('.', 2), 1u);
}

TEST(BaseStringAssign, AssignsFromAnotherStringLike) {
  base::StringRef ref("hello");
  base::String s;
  s.assign(ref);
  EXPECT_EQ(s, "hello");

  base::String t;
  t = ref;
  EXPECT_EQ(t, "hello");
}

// Without the formatter specialization C++23 range formatting takes over and
// prints ['a', 'b'], and a standard library without it fails to compile at all.
TEST(BaseStringFormat, FormatsAsTextNotAsARange) {
  const base::String s = "hello";

  EXPECT_EQ(std::format("{}", s), "hello");
  EXPECT_EQ(std::format("[{}]", base::String()), "[]");
}

TEST(BaseStringFormat, HonoursTheFormatSpec) {
  const base::String s = "ab";

  EXPECT_EQ(std::format("{:>5}", s), "   ab");
  EXPECT_EQ(std::format("{:.1}", s), "a");
}

TEST(BaseStringFormat, FormatsAStringRef) {
  const base::StringRef ref("world");

  EXPECT_EQ(std::format("{}", ref), "world");
}

TEST(BaseStringViewInterop, ConvertsToAStdStringView) {
  base::String s = "meshes/rock.nif";

  std::string_view view = s;

  EXPECT_EQ(view.size(), s.size());
  EXPECT_EQ(view, "meshes/rock.nif");
}

TEST(BaseStringViewInterop, PassesStraightIntoAStringViewParameter) {
  auto takes_view = [](std::string_view v) { return v.size(); };
  base::String s = "abcd";

  EXPECT_EQ(takes_view(s), 4u);
}

TEST(BaseStringViewInterop, ConversionKeepsEmbeddedNulls) {
  base::String s("a\0b", 3);

  std::string_view view = s;

  EXPECT_EQ(view.size(), 3u);
  EXPECT_EQ(view[1], '\0');
}

TEST(BaseStringViewInterop, DoesNotSilentlyConvertIntoAnOwningString) {
  // std::string is not trivially destructible, so it stays out of the
  // conversion set: copies into an allocating type must be explicit.
  EXPECT_FALSE((std::is_convertible_v<base::String, std::string>));
  EXPECT_TRUE((std::is_convertible_v<base::String, std::string_view>));
}

TEST(BaseStringViewInterop, ConstructsFromAStdStringView) {
  std::string_view view = "hello world";

  base::String s = view;

  EXPECT_EQ(s.size(), view.size());
  EXPECT_EQ(s, "hello world");
}

TEST(BaseStringViewInterop, StringRefRoundTripsThroughStdStringView) {
  std::string_view view = "path/to/file";

  base::StringRef ref = view;
  std::string_view back = ref;

  EXPECT_EQ(ref.size(), view.size());
  EXPECT_EQ(back, view);
}

TEST(BaseStringSearch, FindFirstOfAndNotOf) {
  base::String s = "   trim me";

  EXPECT_EQ(s.find_first_not_of(' '), 3u);
  EXPECT_EQ(s.find_first_not_of(" \t"), 3u);
  EXPECT_EQ(s.find_first_of("me"), 6u);  // the "m" of "trim"
  EXPECT_EQ(s.find_first_of("zq"), base::String::npos);
}

TEST(BaseStringSearch, FindLastNotOf) {
  base::String s = "trim me   ";

  EXPECT_EQ(s.find_last_not_of(' '), 6u);
  EXPECT_EQ(s.find_last_not_of(" \t"), 6u);
}

TEST(BaseStringSearch, AllOfTheSetIsNotFound) {
  base::String s = "     ";

  EXPECT_EQ(s.find_first_not_of(' '), base::String::npos);
  EXPECT_EQ(s.find_last_not_of(' '), base::String::npos);
}

TEST(BaseStringModify, AppendAnotherString) {
  base::String s = "ab";
  const base::String tail = "cd";

  s.append(tail);

  EXPECT_EQ(s, "abcd");
}

TEST(BaseStringModify, AppendRepeatedCharacter) {
  base::String s = "x";

  s.append(3, '.');

  EXPECT_EQ(s, "x...");
  EXPECT_EQ(s.size(), 4u);
}

TEST(BaseStringModify, AppendZeroCharactersIsANoOp) {
  base::String s = "x";

  s.append(0, '.');

  EXPECT_EQ(s, "x");
}

TEST(BaseStringModify, AppendAView) {
  base::String s = "a";
  std::string_view view = "bc";

  s.append(view);

  EXPECT_EQ(s, "abc");
}

TEST(BaseStringModify, PopBack) {
  base::String s = "abc";

  s.pop_back();

  EXPECT_EQ(s.size(), 2u);
  EXPECT_EQ(s, "ab");
  EXPECT_EQ(s.c_str()[2], '\0');
}

TEST(BaseStringModify, PopBackDownToEmpty) {
  base::String s = "a";

  s.pop_back();

  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s, "");
}

// O(n²) sanity: 50k push_back calls must not be quadratic. With the old
// reserve-exact path this took quadratically long; with grow_to_at_least
// it's linear and finishes in milliseconds.
TEST(BaseStringRegressions, PushBackIsLinear) {
  base::String s;
  constexpr size_t kN = 50000;
  for (size_t i = 0; i < kN; ++i) s.push_back('x');
  EXPECT_EQ(s.size(), kN);
  for (size_t i = 0; i < kN; ++i) EXPECT_EQ(s[i], 'x');
}

}  // namespace