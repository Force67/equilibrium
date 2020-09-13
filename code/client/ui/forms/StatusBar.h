// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_StatusBar.h"

namespace noda
{
namespace ui
{
  class StatusBar final : public QDockWidget, public Ui::StatusBar
  {
	Q_OBJECT;

  public:
	StatusBar();

	void SetColor(const QT::QColor);
  };
}
}