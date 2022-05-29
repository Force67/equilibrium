// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// The symbol table contains all symbolic information for a given database, such as
// function names, type info etc.

#include <base/logging.h>
#include <program_database/symbol_table.h>

namespace program_database {
void SymbolTable::LoadHeaders(std::unique_ptr<byte> header_data) {
  header_data_ = std::move(header_data);

  byte* ptr = header_data_.get();
  db_header_ = reinterpret_cast<v1::Header*>(ptr);

  LOG_INFO("Loading database with timestamp: {}",
           db_header_->create_date_time_stamp);

  ptr += sizeof(v1::Header);
  seg_header_ = reinterpret_cast<v1::SegmentHeader*>(ptr);

  LOG_INFO("Loading {} segments", seg_header_->num_sections);
}
}  // namespace program_database