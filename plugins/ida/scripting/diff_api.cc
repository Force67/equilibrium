// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Bindings for binary diffing utilities.

#include "script_binding.h"
#include "tools/signature_maker.h"

namespace {
constexpr char kCSVExportName[] = R"(C:\Users\vince\Downloads\out.csv)";

// Note that we have to take int64_t for conversion issues...
// This needs to be done sadly.
// We really should introduce some safe conversion method :(
ScriptBinding<bool, int64_t> s1("CreateAndPrintSignature", [](int64_t ea) {
  SignatureMaker sigMaker;
  return sigMaker.CreateAndPrintSignature(static_cast<ea_t>(ea));
});

// This is more of a hack at the moment.
ScriptBinding<bool, int64_t> s2("HACK_CreateAddExportCSV", [](int64_t in) {
  const ea_t ea = static_cast<ea_t>(in);

  SignatureMaker sigMaker;

  std::string pattern;
  int8_t ofs = 0;
  bool is_data = false;
  auto result = sigMaker.CreateSignature(ea, pattern, ofs, is_data);
  if (result != SignatureMaker::Result::kSuccess) {
    ofs = 0;
    pattern = "UNSET";
  }

  if (FILE* fptr = qfopen(kCSVExportName, "a")) {
    qfprintf(fptr, "HACK_ImportSignature(0x%llx,%s,%d);\n", ea, pattern.c_str(), ofs);
    qfclose(fptr);
  }

  return true;
});

ScriptBinding<bool, int64_t> s3("HACK_ImportSignature", [](int64_t in) {
  const ea_t ea = static_cast<ea_t>(in);

  #if 0
  if (bin_search2(seg->start_ea, address, bytes_.begin(), masks_.begin(),
                  bytes_.size(),
                  seg->start_ea <= address ? BIN_SEARCH_FORWARD
                                           : BIN_SEARCH_BACKWARD) == BADADDR) {
  
  }
  #endif



  return true;
});
}  // namespace