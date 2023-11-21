#include <gtest/gtest.h>
#include "map.h"

namespace base {
namespace {
class MapTest : public ::testing::Test {
 protected:
  base::Map<int, std::string> map;
};

TEST_F(MapTest, InsertAndContains) {
  map.Insert(1, "one");
  EXPECT_TRUE(map.Contains(1));
  EXPECT_FALSE(map.Contains(2));
}

TEST_F(MapTest, DuplicateInsert) {
  map.Insert(1, "one");
  map.Insert(1, "uno");
  // Add checks here based on your implementation's behavior on duplicate
  // inserts
}

TEST_F(MapTest, Erase) {
  map.Insert(1, "one");
  map.Erase(1);
  EXPECT_FALSE(map.Contains(1));
}

TEST_F(MapTest, EraseNonExistentKey) {
  map.Erase(2);
  // Add checks here based on your implementation's behavior when erasing a
  // non-existent key
}
}  // namespace
}  // namespace base
