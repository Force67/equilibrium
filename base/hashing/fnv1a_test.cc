#include "fnv1a.h"

#include "gtest/gtest.h"

class FNV1a64Test : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up test input data and expected output
    test_data_ = {
        {"", 14695981039346656037ull},
        {"a", 12638187200555641996ull},
        {"ab", 620445648566982762ull},
        {"abc", 16654208175385433931ull},
        {"abcd", 18165163011005162717ull},
        {"abcde", 7154184807124264104ull},
        {"abcdef", 15567776504244095498ull},
        {"abcdefg", 4642726675185563447ull},
        {"abcdefgh", 2727646559950394989ull},
        {"abcdefghi", 18100548702444955852ull},
        {"abcdefghij", 13383510251204481554ull},
    };
  }

  // Test data is a vector of pairs, where the first element of each pair is a string
  // and the second element is the expected hash value for that string.
  std::vector<std::pair<std::string, base::hashing::hash_type>> test_data_;
};

TEST_F(FNV1a64Test, TestFNV1a64Function) {
  for (const auto& [str, expected_hash] : test_data_) {
    const base::hashing::hash_type actual_hash =
        base::hashing::FNV1a64(reinterpret_cast<const unsigned char*>(str.c_str()));
    EXPECT_EQ(expected_hash, actual_hash) << "For input string: " << str;
  }
}