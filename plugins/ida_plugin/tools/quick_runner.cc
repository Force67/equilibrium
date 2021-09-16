// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "plugin.h"
#include "quick_runner.h"

#include "bindings/binding.h"
#include "signature_maker.h"
#include "usi_maker.h"

extern Binding<bool, ea_t> MakeSignature;

namespace {
void CreatePrintSignature() {
  SignatureMaker sigMaker;

  std::string signature;
  ptrdiff_t offset = 0;
  bool dumb = false;

  sigMaker.CreateUniqueSignature(get_screen_ea(), signature, offset, false,
                                 dumb);
}
}


QuickRunner::QuickRunner(Plugin& plugin) {
  // glue UI request to runner.
  connect(&plugin.ui(), &PluginUi::RequestFeature, this,
          &QuickRunner::DispatchFeature);
}

QuickRunner::~QuickRunner() {}

void QuickRunner::DispatchFeature(int data) {
  const FeatureIndex code = static_cast<FeatureIndex>(data);
  switch (code) {
    case FeatureIndex::kSignatureMaker:
      CreatePrintSignature();
      break;
    case FeatureIndex::kUSIMaker:
      CreateUSI(get_screen_ea());
      break;
    default:
      LOG_ERROR("??? : {}", data);
      break;
  }

  last_feature_ = code;
}