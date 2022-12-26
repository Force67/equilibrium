#include "gtest/gtest.h"

#include "base/hashing/elf.h"  // The header file for the Elf function

class ElfTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up test input data and expected output
    test_data_ = {{"", 0},
                  {"a", 0x61},
                  {"ab", 0x672},
                  {"abc", 0x6783},
                  {"abcd", 0x67894},
                  {"abcde", 0x6789a5},
                  {"abcdef", 0x6789ab6},
                  {"abcdefg", 0x6789abc7},
                  {"abcdefgh", 0x789abcd8}};
  }

  // Test data is a vector of pairs, where the first element of each pair is a string
  // and the second element is the expected hash value for that string.
  std::vector<std::pair<std::string, base::hashing::hash_type>> test_data_;
};

TEST_F(ElfTest, TestElfFunction) {
  for (const auto& [str, expected_hash] : test_data_) {
    const base::hashing::hash_type actual_hash =
        base::hashing::Elf(reinterpret_cast<const unsigned char*>(str.c_str()));
    EXPECT_EQ(expected_hash, actual_hash) << "For input string: " << str;
  }
}