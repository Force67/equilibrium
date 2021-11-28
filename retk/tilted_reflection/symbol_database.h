// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <vector>
#include <string>

#include "base/io/file/file_path.h"

namespace tilted_reflection {

// TODO for the future: refactor this in a SymbolDatabaseBase class
// with SymbolDatabaseJSON and SymbolDatabaseBin inheriting.
// The decision to store all members in memory will be exclusive to the JSON
// implementation, as the binary db will just keep a db handle instead.
class SymbolDatabase {
 public:
  SymbolDatabase(const base::Path&);
  ~SymbolDatabase();

  using HashType = uint64_t;

  // unique symbol identification.
  struct Record {
    int8_t direct_offset;
    int8_t indirect_offset;
    uint16_t count;
    HashType hash;
    std::string signature;
  };

  void AddSymbol(const Record& record);

  bool LoadSymbols();
  bool StoreSymbols();

  inline void clear() { records_.clear(); }
  inline size_t count() const { return records_.size(); }
  // without spacing a buffer should just require to be
  // char buf[64] bytes big.
  // we should denote with length
 private:
  const base::Path path_;
  std::vector<Record> records_;
};
}  // namespace tilted_reflection