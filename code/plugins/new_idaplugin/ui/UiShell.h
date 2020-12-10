// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class Plugin;

#include "UiStorage.h"

#include <QObject>
#include <QScopedPointer>

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
  void Tick();

  static ssize_t idaapi StaticEvent(void *, int, va_list);

  void HandleEvent(int, va_list);

  void Connect();

  Plugin &_plugin;
  UiStorage _store;

  enum class IdbState {
	Opening,
	Closing,
  };

  QScopedPointer<QLabel> _wastedTime;
  QScopedPointer<QTimer> _timer;

  uint32_t _tick = 0;
};