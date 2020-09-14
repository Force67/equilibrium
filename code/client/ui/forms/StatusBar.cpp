// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "StatusBar.h"

namespace noda::ui
{
  StatusBar::StatusBar() :
      QDockWidget(QApplication::activeWindow())
  {
	setupUi(this);
  }

  void StatusBar::SetColor(const QT::QColor color)
  {
	QPalette pal;
	pal.setColor(QPalette::Background, color);

	QWidget::setPalette(pal);
  }
} // namespace noda::ui