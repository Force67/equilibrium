// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "script_binding.h"
#include "tools/signature_maker.h"

namespace {
// Note that we have to take int64_t for conversion issues...
// This needs to be done sadly.
// We really should introduce some safe conversion method :(
ScriptBinding<bool, int64_t> s1("CreateAndPrintSignature", [](int64_t ea) {
  SignatureMaker sigMaker;
  return sigMaker.CreateAndPrintSignature(static_cast<ea_t>(ea));
});

ScriptBinding<bool, ea_t> s2("CreateExportSignatureCSV", [](ea_t ea) {
  SignatureMaker sigMaker;

  std::string pattern;
  int8_t ofs = 0;
  bool is_data = false;

  sigMaker.CreateSignature(ea, pattern, ofs, is_data);

  return false;
});
}  // namespace