// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_NSyncDialog.h"

namespace noda {
  class NSyncDialog final : public QDialog, public Ui::NSyncDialog {
	Q_OBJECT;

  public:
	NSyncDialog(QWidget *parent);
	~NSyncDialog() = default;
  };
} // namespace noda