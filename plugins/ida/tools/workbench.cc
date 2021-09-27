// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "plugin.h"
#include "workbench.h"

#include "bindings/binding.h"
#include "signature_maker.h"

#include "ui/forms/input_dialog.h"

extern Binding<bool, ea_t> MakeSignature;

namespace {
void CreatePrintSignature(ea_t ea) {
  SignatureMaker sigMaker;

  std::string signature;
  ptrdiff_t offset = 0;
  bool dumb = false;

  sigMaker.CreateUniqueSignature(ea, signature, offset, false, dumb);
}
}  // namespace

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

#if 0
void Workbench::ProduceUSI(const ea_t ea) {
  // This should be moved somewhere else, such as a WorkSpace::Init Sub
  if (data_.sym_table_path.empty()) {
    forms::InputDialog dialog("Path selection", "Enter path to symbol table");
    connect(&dialog, &forms::InputDialog::OnConfirm,
            [&](const QString& new_string) {
              data_.sym_table_path = new_string.toUtf8().data();
            });

    if (dialog.exec() != QDialog::Accepted) {
      LOG_ERROR("No path set for symbol table!");
      data_.sym_table_path.clear();
      return;
    }
  }

  usi_factory_.Create(ea);
}
#endif

void Workbench::RunFeature(int data) {
  const ea_t screen_ea = get_screen_ea();
  const FeatureIndex code = static_cast<FeatureIndex>(data);

  switch (code) {
    case FeatureIndex::kSignatureMaker:
      CreatePrintSignature(screen_ea);
      break;
    case FeatureIndex::kUSIMaker:
      reflection_.AddSymbol(screen_ea);
      break;
    default:
      LOG_ERROR("??? : {}", data);
      break;
  }

  last_feature_ = code;
}