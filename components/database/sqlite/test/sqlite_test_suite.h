// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <sqlite/sqlite_db.h>
#include <base/filesystem/scoped_temp_dir.h>

namespace database {

class SqliteTestSuite : public ::testing::Test {
 public:
  void SetUp() override;
  void TearDown() override;

  SqliteDb& db() { return db_; }
  base::Path db_path();
 private:
  SqliteDb db_;
  base::ScopedTempDir temp_dir_;
};
}  // namespace database