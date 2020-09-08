// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_StatusBar.h"

// backport of QColorConstants for QT 5.6
// colors taken from official color spec at
// https://doc.qt.io/qt-5/qcolorconstants.html
namespace colorconstant {
  constexpr QRgb orange{ 0xffa500 };
  constexpr QRgb green{ 0x008000 };
  constexpr QRgb red{ 0xff0000 };
} // namespace colorconstant

class UiStatusBar final : public QDockWidget, public Ui::StatusBar {
  Q_OBJECT;

  public:
  UiStatusBar();

  void SetColor(const QT::QColor);
};
