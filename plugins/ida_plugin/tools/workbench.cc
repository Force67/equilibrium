// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "plugin.h"
#include "workbench.h"

#include "bindings/binding.h"
#include "signature_maker.h"
#include "usi_maker.h"

extern Binding<bool, ea_t> MakeSignature;

namespace {
void CreatePrintSignature(ea_t ea) {
  SignatureMaker sigMaker;

  std::string signature;
  ptrdiff_t offset = 0;
  bool dumb = false;

  sigMaker.CreateUniqueSignature(ea, signature, offset, false,
                                 dumb);
}
}

// TODO: replace me with utility class
static Workbench* work_bench_instance{nullptr};

Workbench* work_bench() {
  return work_bench_instance;
}

Workbench::Workbench(Plugin& plugin) {
  work_bench_instance = this;

  // glue UI request to runner.
  connect(&plugin.ui(), &PluginUi::RequestFeature, this,
          &Workbench::RunFeature);
}

Workbench::~Workbench() {
  work_bench_instance = nullptr;
}

void Workbench::RunFeature(int data) {
  const ea_t screen_ea = get_screen_ea();
  const FeatureIndex code = static_cast<FeatureIndex>(data);

  switch (code) {
    case FeatureIndex::kSignatureMaker:
      CreatePrintSignature(screen_ea);
      break;
    case FeatureIndex::kUSIMaker:
      usi_factory_.Create(screen_ea);
      break;
    default:
      LOG_ERROR("??? : {}", data);
      break;
  }

  last_feature_ = code;
}