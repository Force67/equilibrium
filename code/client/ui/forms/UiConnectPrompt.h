// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "ui/moc/ui_UiConnectPromt.h"

class UiController;

class UiConnectPromt final : public QDialog, public Ui::ConnectPromt
{
  Q_OBJECT;

public:
  UiConnectPromt(UiController &);

private:
  void OnConnect();
  void SetDontAskagain();

  UiController &_controller;
};
