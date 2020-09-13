// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#include "UiStatusBar.h"

UiStatusBar::UiStatusBar() :
    QDockWidget(QApplication::activeWindow())
{
  setupUi(this);
}

void UiStatusBar::SetColor(const QT::QColor color)
{
  QPalette pal;
  pal.setColor(QPalette::Background, color);

  QWidget::setPalette(pal);
}