// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "ui/moc/ui_StatusBar.h"

class UiStatusBar final : public QDockWidget, public Ui::StatusBar {
  Q_OBJECT;

public:
  UiStatusBar() : QDockWidget(QApplication::activeWindow()) { 
	  setupUi(this); 
  }

  void SetColor(const char *name);
};
