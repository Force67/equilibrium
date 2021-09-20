// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_input_dialog.h"

namespace forms {
class UiController;

class InputDialog final : public QDialog, public Ui::InputDialog {
  Q_OBJECT;

 public:
  InputDialog();

 private:
  void OnConnect();
};
}  // namespace forms