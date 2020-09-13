// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Plugin.h"
#include <qsettings.h>
#include "ui/moc/ui_Settings.h"

namespace noda
{
namespace ui
{
  class Settings final : public QDialog, public Ui::UiSettings
  {
	Q_OBJECT;

  public:
	Settings(SyncClient &, QWidget *);
	~Settings() = default;

  private:
	void OnIpChange(const QString &);
	void OnPortChange(const QString &);
	void OnPasswordChange(const QString &);
	void OnNameChange(const QString &);

	QSettings _settings;
	SyncClient &_client;
  };
}
}