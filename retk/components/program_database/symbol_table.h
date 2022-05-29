// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// The symbol table contains all symbolic information for a given database, such as
// function names, type info etc.
#pragma once

#include <memory>
#include <base/arch.h>
#include <base/containers/span.h>
#include <base/numeric_limits.h>
#include <program_database/string_pool.h>
#include <program_database/database_spec.h>

#include <vector>

namespace program_database {

class StringTable {
 public:
  v1::NameRecord AddName(base::StringRef ansii_string) {
    v1::NameRecord new_record{};

    // fetching a namerecord is more expensive, so for tiny data we store it in place
    if (ansii_string.length() <= 8) {
      std::strncpy(new_record.name, ansii_string.data(), 8);
      return new_record;
    }

    new_record = {.whole = base::MinMax<u64>::max()};
    // TODO: somehow keep track...

    return new_record;
  }

 private:
  std::vector<v1::NameRecord> queued_records_;
  StringPool<char> ansii_strings_;
  StringPool<wchar_t> wide_strings_;
};

class SymbolTable {
 public:
  void LoadHeaders(std::unique_ptr<byte> header_data);

 private:
  StringPool<char> ansii_strings_;
  StringPool<wchar_t> wide_strings_;

  v1::Header* db_header_{nullptr};
  v1::SegmentHeader* seg_header_{nullptr};
  std::unique_ptr<byte> header_data_;
};
}  // namespace program_database