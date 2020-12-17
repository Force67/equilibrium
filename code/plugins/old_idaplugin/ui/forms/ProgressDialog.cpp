// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ProgressDialog.h"
#include "Pch.h"

namespace noda {

ProgressDialog::ProgressDialog(QWidget* parent,
                               const QString& title,
                               const QString& desc)
    : QDialog(parent) {
  setupUi(this);
  setWindowTitle(title);
  labelDesc->setText(desc);
}

void ProgressDialog::SetProgress(int value, const QString& status) {
  barProgress->setValue(value);
  labelStatus->setText(status);
}
}  // namespace noda