// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class Plugin;

#include "UiStorage.h"
#include <QObject>

namespace QT {
  class QAction;
  class QLabel;
  class QTimer;
  class QWidget;
  class QMainWindow;
} // namespace QT

// returns the currently most top widget
QT::QWidget *GetTopWidget();

// Returns the main application window
QT::QMainWindow *GetMainWindow();

class UiShell final : public QObject {
  Q_OBJECT;

public:
  explicit UiShell(Plugin &);
  ~UiShell();

private:
  static ssize_t idaapi UiEvent(void *, int, va_list);

  void Connect();

  Plugin &_plugin;
  UiStorage _store;
};