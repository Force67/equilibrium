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
  const typename BaseStringType::character_type* c_str =
      this->GetTestSentence();
  BaseStringType base_str_from_c_str(c_str);
  StdStringType std_str_from_c_str(c_str);
  ASSERT_EQ(base_str_from_c_str.size(), std_str_from_c_str.size());
  ASSERT_EQ(base_str_from_c_str.compare(std_str_from_c_str), 0);

  // Copy construction
  BaseStringType base_str_copy(base_str_from_c_str);
  ASSERT_EQ(base_str_copy.size(), base_str_from_c_str.size());
  ASSERT_EQ(base_str_copy.compare(base_str_from_c_str), 0);
}

TYPED_TEST(BaseStringTest, Assignment) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Assignment from C-style string
  const typename BaseStringType::character_type* c_str =
      this->GetTestSentence();
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
  const typename BaseStringType::character_type* c_str =
      this->GetTestSentence();
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

TYPED_TEST(BaseStringTest, Concatenation) {
  using BaseStringType = typename TestFixture::BaseStringType;
  using StdStringType = typename TestFixture::StdStringType;

  // Concatenation with C-style string
  const typename BaseStringType::character_type* c_str =
      this->GetTestSentence();
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
}  // namespace