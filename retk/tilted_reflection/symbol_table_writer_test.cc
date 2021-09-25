// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "symbol_table_writer.h"
#include <gtest/gtest.h>

namespace tilted_reflection {
namespace {
TEST(SymbolTableWriter, WriteEntry) {
  SymbolTableWriter writer;

  writer.JustCreate();

  SymbolTableWriter::CreateInfo info;
  info.count = 1;
  info.direct_offset = 10;
  info.indirect_offset = 20;
  info.kind = SymbolTableWriter::kFunction;
  info.signature = "AA BB CC";
  writer.CreateEntry("void MyFunc(void*)", "MyFunc(void*)", info);

  std::puts(writer.DocToString().c_str());
}
}  // namespace
}  // namespace tilted_reflection