// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_RunDialog.h"

namespace noda {
namespace ui {
  class RunDialog final : public QDialog, public Ui::RunDialog {
	Q_OBJECT;

  public:
	RunDialog(QWidget *parent);
	~RunDialog() = default;
  };
}
} // namespace noda::ui