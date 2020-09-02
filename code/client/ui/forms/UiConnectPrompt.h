// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "ui/moc/ui_UiConnectPromt.h"

class UiConnectPromt final : public QDialog, public Ui::ConnectPromt {
  Q_OBJECT;

public:
  UiConnectPromt() : QDialog(QApplication::activeWindow()) { 
	  setupUi(this); 
	  setWindowIcon(QIcon(":/logo"));
  }
};
