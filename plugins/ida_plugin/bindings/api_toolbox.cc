// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <expr.hpp>
#include <bindings/api_toolbox.h>
#include "tools/toolbox.h"

namespace binding {
namespace {
using namespace tools;

error_t idaapi MakeSignature(idc_value_t* argv, idc_value_t* res) {
  const ea_t target_address = argv[0].num;

  std::string result;
  ptrdiff_t offset = 0;
  bool dumb = false;
  toolbox()->signature_generator().UniqueSignature(target_address, result,
                                                   offset, false, dumb);

  return eOk;
}

// because yamashi is a lazy:
error_t idaapi MakeSignature2(idc_value_t* argv, idc_value_t* res) {
  const char* target_name = argv[0].c_str();
  const ea_t target_address = argv[1].num;
  auto& generator = tools::toolbox()->signature_generator();

  auto writeEntry = [](const std::string& str) {
    if (FILE* file =
            qfopen(R"(C:\Users\vince\Documents\Development\yamashiislazy.txt)",
                   "a+")) {
      qfwrite(file, str.c_str(), str.length());
      qfclose(file);
    }
  };

  std::string result = "";
  ptrdiff_t offset = 0;
  bool dumb_flag = false;
  if (generator.UniqueSignature(target_address, result, offset, false,
                                dumb_flag) ==
      tools::SignatureGenerator::Result::kSuccess) {
    std::string stupid_string;

    if (dumb_flag)
      stupid_string = "TiltedPhoques::Pattern::kRelativeIndirection4";
    else
      stupid_string = "TiltedPhoques::Pattern::kDirect";

    auto buf =
        fmt::format(R"(RegisterPointer("{}", Pattern("{}", {}, {}, {}, {})))"
                    ";\n",
                    target_name, result, stupid_string, 1, offset, 1);

    writeEntry(buf.c_str());
  } else {
    auto buf = fmt::format(
        R"(RegisterPointer({}, Pattern("PLEASESETME_{:x}", 1, PatternType, Offset, 1)))"
        ";\n",
        target_name, target_address);

    writeEntry(buf.c_str());
  }

  return eOk;
}

// this definitly needs some nice cxx wrapper.
const char MakeSignatureArgs[] = {VT_INT64, VT_STR, 0};
const char MakeSignatureArgs2[] = {VT_STR, VT_INT64, 0};

const ext_idcfunc_t MakeSignatureDesc = {
    "TK_MakeSignature", MakeSignature, MakeSignatureArgs, nullptr, 0, 0};

const ext_idcfunc_t MakeSignatureDesc2 = {
    "TK_MakeSignature2", MakeSignature2, MakeSignatureArgs2, nullptr, 0, 0};
}  // namespace

void RegisterToolboxApi() {
  add_idc_func(MakeSignatureDesc);
  add_idc_func(MakeSignatureDesc2);
}

void DestroyToolboxApi() {
  del_idc_func(MakeSignatureDesc.name);
  del_idc_func(MakeSignatureDesc2.name);
}
}  // namespace binding