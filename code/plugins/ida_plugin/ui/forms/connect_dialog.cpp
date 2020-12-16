// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "connect_dialog.h"
#include <qsettings.h>

namespace forms {
ConnectDialog::ConnectDialog(UiController& controller)
    : QDialog(QApplication::activeWindow()), _controller(controller) {
  setupUi(this);

  connect(buttonConnect, &QPushButton::clicked, this,
          &ConnectDialog::OnConnect);
  connect(buttonCancel, &QPushButton::clicked, this, &QDialog::close);
  connect(buttonNO, &QCheckBox::clicked, [](bool checked) {
    QSettings settings;
    settings.setValue("Nd_UiSkipConnect", checked);
  });
}

void ConnectDialog::OnConnect() {
  // we cannot close here yet, due to the lack of context
  QDialog::hide();
  //_controller.ToggleConnect();
  QDialog::accept();
}

bool ConnectDialog::ShouldShow() {
  QSettings settings;
  return !settings.value("Nd_UiSkipConnect", true).toBool();
}
}  // namespace forms