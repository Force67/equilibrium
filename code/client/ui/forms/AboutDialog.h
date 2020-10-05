// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_AboutDialog.h"

namespace noda {
  class AboutDialog final : public QDialog, public Ui::AboutDialog {
	Q_OBJECT;

  public:
	AboutDialog(QWidget *parent);
	~AboutDialog() = default;
  };
} // namespace noda::ui