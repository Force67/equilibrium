#include "gtest/gtest.h"

#include "base/hashing/djb2.h"  // The header file for the djb2 function

class Djb2Test : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up test input data and expected output
    test_data_ = {
        {"", 0ull},
        {"a", 733599527ull},
        {"ab", 24209185930ull},
        {"abc", 798903541326ull},
        {"abcd", 26363817273491ull},
        {"abcde", 870005970439033ull},
        {"abcdef", 28710197024906016ull},
        {"abcdefg", 947436501822320552ull},
        {"abcdefgh", 12818660486427452721ull},
        {"abcdefghi", 17187426430496234459ull},
        {"abcdefghij", 13782749995089622982ull},
    };
  }

  // Test data is a vector of pairs, where the first element of each pair is a string
  // and the second element is the expected hash value for that string.
  std::vector<std::pair<std::string, base::hashing::hash_type>> test_data_;
};

TEST_F(Djb2Test, TestDjb2Function) {
  for (const auto& [str, expected_hash] : test_data_) {
    const base::hashing::hash_type actual_hash =
        base::hashing::djb2(reinterpret_cast<const unsigned char*>(str.c_str()));
    EXPECT_EQ(expected_hash, actual_hash) << "For input string: " << str;
  }
}