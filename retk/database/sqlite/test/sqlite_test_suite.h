// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <sqlite/sqlite_db.h>

namespace database {

class SqliteTestSuite : public ::testing::Test {
 public:
  SqliteDb& db() { return db_; }

  void SetUp() override;
  void TearDown() override;

 private:
  SqliteDb db_;
};
}  // namespace database