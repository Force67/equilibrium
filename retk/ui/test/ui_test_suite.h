// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/files/scoped_temp_dir.h>

namespace database {

class SqliteTestSuite : public ::testing::Test {
 public:
  void SetUp() override;
  void TearDown() override;

 private:
};
}  // namespace database