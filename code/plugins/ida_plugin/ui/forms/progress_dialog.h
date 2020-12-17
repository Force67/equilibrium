// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_progress_dialog.h"

namespace forms {
class ProgressDialog final : public QDialog, public Ui::ProgressDialog {
  Q_OBJECT;

 public:
  explicit ProgressDialog(QWidget* parent,
                          const QString& title,
                          const QString& desc);
  ~ProgressDialog() = default;

  void SetProgress(int value, const QString& status);
};
}  // namespace forms