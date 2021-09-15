// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class Plugin;

#include "idb_storage/ui_data.h"

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
class AddressBookView;
}

// returns the currently most top widget
QT::QWidget* GetTopWidget();

// Returns the main application window
QT::QMainWindow* GetMainWindow();

class PluginUi final : public QObject {
  Q_OBJECT;

 public:
  explicit PluginUi(Plugin&);
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
  void RequestFeature(int featureCode);

 private:
  void Tick();
  void ClenseTheShell();

  static ssize_t idaapi StaticEvent(void*, int, va_list);
  void HandleEvent(int, va_list);

  Plugin& plugin_;
  UiData data_;
  ShellState state_{ShellState::NO_DB};

  std::unique_ptr<QLabel> wastedTime_;
  std::unique_ptr<QTimer> timer_;
  std::unique_ptr<forms::StatusWidget> statusForm_;
  std::unique_ptr<forms::AddressBookView> addressView_;

  QAction* cnAct_ = nullptr;
  QAction* stAct_ = nullptr;
};