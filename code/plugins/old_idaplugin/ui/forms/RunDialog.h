// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_RunDialog.h"

namespace noda {
class RunDialog final : public QDialog, public Ui::RunDialog {
  Q_OBJECT;

 public:
  RunDialog(QWidget* parent);
  ~RunDialog() = default;
};
}  // namespace noda