// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>
#include "WelcomeDialog.h"

namespace noda::ui
{
  WelcomeDialog::WelcomeDialog() :
      QDialog(QApplication::activeWindow())
  {
	setupUi(this);

	connect(buttonContinue, &QPushButton::clicked, this, &QDialog::accept);
	connect(buttonDontShowAgain, &QCheckBox::clicked, [](bool checked) {
	  QSettings settings;
	  settings.setValue("Nd_UiSkipWelcome", !checked);
	});
  }

  bool WelcomeDialog::ShouldShow()
  {
	QSettings settings;
	return settings.value("Nd_UiSkipWelcome", true).toBool();
  }
} // namespace noda::ui