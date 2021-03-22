// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class IdaPlugin;

#include "storage/ui_data.h"

#include <QObject>
#include <QScopedPointer>

namespace QT {
class QAction;
class QLabel;
class QTimer;
class QWidget;
class QMainWindow;
}  // namespace QT

namespace forms {
class StatusWidget;
}

// returns the currently most top widget
QT::QWidget* GetTopWidget();

// Returns the main application window
QT::QMainWindow* GetMainWindow();

class PluginUi final : public QObject {
  Q_OBJECT;

 public:
  explicit PluginUi(IdaPlugin&);
  ~PluginUi();

  enum class ShellState {
    NO_DB,
    IN_DB,
  };

  void SetShellState(ShellState);
  ShellState GetShellState() const;

  void RunFeature();
 signals:
  void ShellStateChange(ShellState newState);

 private:
  void Tick();
  void ClenseTheShell();

  static ssize_t idaapi StaticEvent(void*, int, va_list);
  void HandleEvent(int, va_list);

  IdaPlugin& _plugin;
  UiData data_;
  ShellState _state{ShellState::NO_DB};

  enum class IdbState {
    Opening,
    Closing,
  };

  QScopedPointer<QLabel> _wastedTime;
  QScopedPointer<QTimer> _timer;

  QScopedPointer<forms::StatusWidget> _statusForm;
  QAction* _cnAct = nullptr;
  QAction* _stAct = nullptr;
};