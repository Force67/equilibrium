// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qsettings.h>
#include "Plugin.h"
#include "ui/generated/ui_Settings.h"

namespace forms {
class Settings final : public QDialog, public Ui::SettingsDialog {
  Q_OBJECT;

 public:
  Settings(bool, QWidget*);
  ~Settings() = default;

 private:
  QSettings _settings;
};
}  // namespace forms