// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "Plugin.h"
#include <qsettings.h>
#include "ui/moc/ui_UiSettings.h"

class UiSettings final : public QDialog, public Ui::UiSettings
{
  Q_OBJECT;

public:
  UiSettings(SyncClient &, QWidget *);
  ~UiSettings() = default;

private:
  void OnIpChange(const QString &);
  void OnPortChange(const QString &);
  void OnPasswordChange(const QString &);
  void OnNameChange(const QString &);

  QSettings _settings;
  SyncClient &_client;
};