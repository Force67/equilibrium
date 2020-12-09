// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Plugin.h"
#include <qsettings.h>
#include "ui/generated/ui_Settings.h"

namespace forms {
  class Settings final : public QDialog, public Ui::SettingsDialog {
	Q_OBJECT;

  public:
	Settings(bool, QWidget *);
	~Settings() = default;

  private:
	QSettings _settings;
  };
} // namespace forms