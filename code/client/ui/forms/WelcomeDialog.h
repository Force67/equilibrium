// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_WelcomeDialog.h"

namespace noda {
namespace ui {
  class WelcomeDialog final : public QDialog, public Ui::WelcomeDialog {
	Q_OBJECT;

  public:
	WelcomeDialog();
	~WelcomeDialog() = default;

	static bool ShouldShow();
  };
}
} // namespace noda::ui