#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "base_string.h"

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

  // Default construction
  BaseStringType base_str;
  ASSERT_EQ(base_str.size(), 0);
  ASSERT_EQ(base_str.capacity(), 0);

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

  // Compare with the same string
  ASSERT_EQ(str1.compare(0, str1.size(), str1), 0);

  // Compare with a substring
  ASSERT_EQ(str1.compare(7, 5, str2), 0);

  // Substring starts outside the string
  ASSERT_EQ(str1.compare(14, 1, str2), -1);

  // Substring length exceeds string length
  ASSERT_EQ(str1.compare(7, 10, str2), 1);

  // String is shorter than substring
  ASSERT_EQ(str1.compare(0, 20, str2), -1);

  // Empty substring
  ASSERT_EQ(str1.compare(0, 0, str2), 0);

  // Different strings
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
}  // namespace