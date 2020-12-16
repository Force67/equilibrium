// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_connect_dialog.h"

namespace forms {
class UiController;

class ConnectDialog final : public QDialog, public Ui::ConnectPromt {
  Q_OBJECT;

 public:
  ConnectDialog(UiController&);

  static bool ShouldShow();

 private:
  void OnConnect();

  UiController& _controller;
};
}  // namespace forms