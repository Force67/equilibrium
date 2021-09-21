// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "input_dialog.h"
#include "ui/plugin_ui.h"

namespace forms {
InputDialog::InputDialog(const QString& title, const QString& desc) {
  setupUi(this);
  label->setText(desc);
  setWindowTitle(title);

  connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);
  connect(buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

  connect(this, &QDialog::accept,
          [&]() { OnConfirm(lineEdit->text()); });
}

}  // namespace forms