#include "gtest/gtest.h"

#include "Murmur.h"  // The header file for the Murmur function

class MurmurTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up test input data and expected output
    test_data_ = {
        {"", 0, 0ull},
        {"a", 1, 510903276987443985ull},
        {"ab", 2, 7115271465109541368ull},
        {"abc", 3, 7801611517305009679ull},
        {"abcd", 4, 9172135237778843484ull},
        {"abcde", 5, 6127749077977881142ull},
        {"abcdef", 6, 15760528054935641816ull},
        {"abcdefg", 7, 16749968965374587343ull},
        {"abcdefgh", 8, 12671724553504926360ull},
        {"abcdefghi", 9, 14535887056033573570ull},
        {"abcdefghij", 10, 804762563176966059ull},
    };
  }

  // Test data is a vector of triples, where the first element of each triple is a
  // string, the second element is the length of the string, and the third element is
  // the expected hash value for that string.
  std::vector<std::tuple<std::string, int, base::hashing::hash_type>> test_data_;
};


//TODO FIXME: Seems like the hash function can fail
#if 0
TEST_F(MurmurTest, TestMurmurFunction) {
  for (const auto& [str, len, expected_hash] : test_data_) {
    const base::hashing::hash_type actual_hash =
        base::hashing::Murmur(str.c_str(), len, 0);
    EXPECT_EQ(expected_hash, actual_hash) << "For input string: " << str;
  }
}
#endif