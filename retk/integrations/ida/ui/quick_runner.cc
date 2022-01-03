// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ui/quick_runner.h"
#include "diffing/pattern_generator.h"

namespace {
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
}  // namespace

void RunQuickFeature(forms::RunDialog::FeatureIndex index) {
  const ea_t screen_ea = get_screen_ea();
  switch (index) {
    case forms::RunDialog::FeatureIndex::kTheSignature: {
      PatternGenerator gen;
      gen.CreatePrintSignature(static_cast<ea_t>(screen_ea));
      break;
    }
    case forms::RunDialog::FeatureIndex::kTheUniqueIdentifier:
        // reflection_.AddSymbol(screen_ea);
      break;
    default:
      LOG_ERROR("Invalid feature index ({})", static_cast<int>(index));
  }
}