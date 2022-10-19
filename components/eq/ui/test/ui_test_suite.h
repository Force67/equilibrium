// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/filesystem/scoped_temp_dir.h>

namespace database {

class SqliteTestSuite : public ::testing::Test {
 public:
  void SetUp() override;
  void TearDown() override;

 private:
};
}  // namespace database