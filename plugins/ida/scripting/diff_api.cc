// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Bindings for binary diffing utilities.

#include "script_binding.h"
#include "tools/signature_maker.h"

#include <base/check.h>
#include <bytes.hpp>

namespace {
constexpr char kCSVExportName[] =
    R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\patterns.csv)";
constexpr char kFinalCSVExportName[] =
    R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\mapping.csv)";

// Note that we have to take int64_t for conversion issues...
// This needs to be done sadly.
// We really should introduce some safe conversion method :(
// Maybe ida really wants a string as 'the' parameter
ScriptBinding<0, bool, int64_t> s1("CreateAndPrintSignature", [](int64_t ea) {
  SignatureMaker sigMaker;
  return sigMaker.CreateAndPrintSignature(static_cast<ea_t>(ea));
});

static char EncodeReferenceType(SignatureMaker::ReferenceType type) {
  switch (type) {
    case SignatureMaker::ReferenceType::kDirect:
      return 'D';
    case SignatureMaker::ReferenceType::kRef3:
      return 'R';
  }

  TK_BUGCHECK(false);
  return 0;
}

// This is more of a hack at the moment.
ScriptBinding<1, bool, int64_t> s2("HACK_CreateAddExportCSV", [](int64_t in) {
  const ea_t ea = static_cast<ea_t>(in);

  SignatureMaker sigMaker;

  std::string pattern;
  int8_t ofs = 0;
  bool is_data = false;
  SignatureMaker::ReferenceType rt;

  auto result = sigMaker.CreateSignature(ea, pattern, ofs, is_data, rt);
  if (result != SignatureMaker::Result::kSuccess) {
    ofs = 0;
    pattern = "UNSET";
  }

  if (FILE* fptr = qfopen(kCSVExportName, "a")) {
    qfprintf(fptr, "0x%llx,%s,%d,%c\n", ea, pattern.c_str(), ofs,
             EncodeReferenceType(rt));
    qfclose(fptr);
  }

  return true;
});

ScriptBinding<2, bool, int64_t, const char*, int64_t> s3(
    "HACK_ImportSignature",
    [](int64_t in, const char* pattern, int64_t offset) {
      const ea_t source_ea = static_cast<ea_t>(in);

      if (strcmp(pattern, "UNSET") == 0) {
        if (FILE* fptr = qfopen(kFinalCSVExportName, "a")) {
          qfprintf(fptr, "0x%llx,0x%llx\n", source_ea, "UNSET");
          qfclose(fptr);

          return true;
        }

        return false;
      }

      for (const segment_t* seg = get_first_seg(); seg != nullptr;
           seg = get_next_seg(seg->start_ea)) {
        // patterns may only exist within code segments.
        if (seg->type != SEG_CODE)
          continue;

        compiled_binpat_vec_t vec;
        parse_binpat_str(&vec, seg->start_ea, pattern, 16);

        const ea_t needle =
            bin_search2(seg->start_ea, seg->end_ea, vec, BIN_SEARCH_FORWARD);

        if (needle != BADADDR) {
          if (FILE* fptr = qfopen(kFinalCSVExportName, "a")) {
            qfprintf(fptr, "0x%llx,0x%llx\n", source_ea, needle);
            qfclose(fptr);
          }

          return true;
        }
      }

      return false;
    });
}  // namespace