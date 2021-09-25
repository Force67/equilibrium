// Copyright (C) Force67 2021 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_run_dialog.h"
#include "idb/ui_data.h"

namespace forms {
class RunDialog final : public QDialog, public Ui::RunDialog {
  Q_OBJECT;

 public:
  RunDialog(QWidget* parent, UiData&);
  ~RunDialog() = default;

  int SelectedIndex() const { return data_.last_run_index; }

 private:
  void OnClickOK();
  void SetSelectedItem(int);

 private:
  UiData& data_;
};
}  // namespace forms