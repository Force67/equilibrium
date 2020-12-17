// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_NSyncDialog.h"

namespace noda {
class NSyncDialog final : public QDialog, public Ui::NSyncDialog {
  Q_OBJECT;

 public:
  NSyncDialog(QWidget* parent);
  ~NSyncDialog() = default;
};
}  // namespace noda