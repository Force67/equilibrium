// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_run_dialog.h"

namespace forms {
class RunDialog final : public QDialog, public Ui::RunDialog {
  Q_OBJECT;

 public:
  RunDialog(QWidget* parent);
  ~RunDialog() = default;

  int SelectedIndex() const { return current_index_; }

 private:
  void OnClickOK();
  void ResetSelection();

 private:
  int current_index_ = 0;
};
}  // namespace forms