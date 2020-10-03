// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Plugin.h"
#include <qsettings.h>
#include "ui/moc/ui_Settings.h"

namespace noda {
  namespace sync {
	class SyncController;
  }

  namespace ui {
	class Settings final : public QDialog, public Ui::SettingsDialog {
	  Q_OBJECT;

	public:
	  Settings(bool, QWidget *);
	  ~Settings() = default;

	private:
	  QSettings _settings;
	};
  } // namespace ui
} // namespace noda