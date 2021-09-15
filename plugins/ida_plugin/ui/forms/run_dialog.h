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

  enum class Index {
    kNone,
    kMakeSig,
    kMakeUSI,
  };
  Index SelectedIndex() const;

 private:
  void OnClickOK();
  void ResetSelection();

 private:
  Index current_index_{Index::kNone};
};
}  // namespace forms