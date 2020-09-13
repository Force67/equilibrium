// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>
#include "ConnectDialog.h"
#include "ui/UiController.h"

namespace noda::ui
{
  ConnectDialog::ConnectDialog(UiController &controller) :
      QDialog(QApplication::activeWindow()), _controller(controller)
  {
	setupUi(this);

	connect(buttonConnect, &QPushButton::clicked, this, &ConnectDialog::OnConnect);
	connect(buttonCancel, &QPushButton::clicked, this, &QDialog::close);
	connect(buttonNO, &QCheckBox::clicked, [](bool checked) {
	  QSettings settings;
	  settings.setValue("Nd_UiSkipConnect", !checked);
	});
  }

  void ConnectDialog::OnConnect()
  {
	_controller.ToggleConnect();
	QDialog::accept();
  }

  bool ConnectDialog::ShouldShow()
  {
	QSettings settings;
	return settings.value("Nd_UiSkipConnect", true).toBool();
  }
}