// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "binding.h"
#include "tools/ida_signature_maker.h"

namespace {
using namespace retk;

Binding<bool, ea_t> MakeSignature("MakeSignature", [](ea_t ea) {
  IDASignatureMaker sigMaker;

  std::string signature;
  ptrdiff_t offset = 0;
  bool dumb = false;

  return sigMaker.CreateUniqueSignature(ea, signature, offset, false, dumb) ==
         IDASignatureMaker::Result::kSuccess;
});
}