// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/generated/ui_WelcomeDialog.h"

namespace noda {
class WelcomeDialog final : public QDialog, public Ui::WelcomeDialog {
  Q_OBJECT;

 public:
  WelcomeDialog();
  ~WelcomeDialog() = default;

  static bool ShouldShow();
};
}  // namespace noda