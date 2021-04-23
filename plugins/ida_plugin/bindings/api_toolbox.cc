// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <expr.hpp>
#include <bindings/api_toolbox.h>
#include <tools/signature/sigmaker.h>

namespace binding {
namespace {

error_t idaapi MakeSignature(idc_value_t* argv, idc_value_t* res) {
  tools::GenerateSignature(argv[0].num);
  // argv[1].c_str();

  return eOk;
}

error_t idaapi MakeSignature2(idc_value_t* argv, idc_value_t* res) {
  tools::GenerateSignature(argv[1].num);
  // argv[1].c_str();

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