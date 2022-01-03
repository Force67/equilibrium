// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <string>
#include "symbol_database.h"
#include <gtest/gtest.h>

#include <base/filesystem/scoped_temp_dir.h>

namespace tilted_reflection {
namespace {
class SymbolDatabaseTest : public ::testing::Test {
 public:
  void SetUp() override { ASSERT_TRUE(temp_dir_.CreateUniqueTempDir()); }
  void TearDown() override {}

  base::Path db_path() {
    return temp_dir_.path() / "test.json";
  }

 private:
  base::ScopedTempDir temp_dir_;
};

TEST_F(SymbolDatabaseTest, WriteEntries) {
  SymbolDatabase database(db_path());

  // lay down 10 initial entries.
  for (int i = 0; i < 10; i++) {
    SymbolDatabase::Record r;
    r.count = i;
    r.hash = i; // hashes must be unique
    r.direct_offset = 20;
    r.indirect_offset = 30;
    r.signature = "AA BB CC";
    database.AddSymbol(r);
  }

  EXPECT_EQ(database.count(), 10);
  EXPECT_TRUE(database.StoreSymbols());

  // drain memory cache
  database.clear();
  EXPECT_TRUE(database.LoadSymbols());
  EXPECT_EQ(database.count(), 10);
}
}  // namespace
}  // namespace tilted_reflection