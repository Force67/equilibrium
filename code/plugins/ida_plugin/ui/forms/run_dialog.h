// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "tools/feature_dispatch.h"
#include "ui/generated/ui_run_dialog.h"

namespace forms {
class RunDialog final : public QDialog, public Ui::RunDialog {
  Q_OBJECT;

 public:
  RunDialog(QWidget* parent);
  ~RunDialog() = default;

  features::FeatureIndex SelectedIndex() const;

 private:
  void OnClickOK();
  void ResetSelection();

  int _index = 0;
};
}  // namespace forms