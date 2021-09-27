// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <string>
#include "symbol_database.h"
#include <gtest/gtest.h>

#include <base/files/scoped_temp_dir.h>

namespace tilted_reflection {
namespace {
class SymbolDatabaseTest : public ::testing::Test {
 public:
  void SetUp() override { ASSERT_TRUE(temp_dir_.CreateUniqueTempDir()); }
  void TearDown() override {}

  base::fs::path db_path() { return temp_dir_.Path() / "test.json"; }

 private:
  base::ScopedTempDir temp_dir_;
};

TEST_F(SymbolDatabaseTest, WriteEntries) {
  SymbolDatabase database(db_path().u8string().c_str());

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

  SymbolDatabase::Record r;
  r.count = 11;
  r.hash = 20;
  r.direct_offset = 30;
  r.indirect_offset = 40;
  r.signature = "BB CC DD";
  database.AddSymbol(r);
  EXPECT_EQ(database.count(), 11);
}
}  // namespace
}  // namespace tilted_reflection