// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "ui/moc/ui_StatusBar.h"

QT_BEGIN_NAMESPACE

class UiStatusBar final : public QDockWidget, public Ui_StatusBar {
  Q_OBJECT;

public:
  UiStatusBar() : QDockWidget(QApplication::activeWindow()) { 
	  setupUi(this);
  }
};

QT_END_NAMESPACE