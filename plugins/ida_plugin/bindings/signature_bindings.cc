// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "binding.h"
#include "tools/signature_maker.h"

Binding<bool, ea_t> MakeSignature("MakeSignature", [](ea_t ea) {
  SignatureMaker sigMaker;

  std::string signature;
  ptrdiff_t offset = 0;
  bool dumb = false;

  return sigMaker.CreateUniqueSignature(ea, signature, offset, false, dumb) ==
         SignatureMaker::Result::kSuccess;
});