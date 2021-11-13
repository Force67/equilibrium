// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class Plugin;

#include "idb/ui_data.h"
#include "utils/ida_plus.h"

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

class PluginUi final : public QObject, public ial::UiEventHandler {
  Q_OBJECT;

 public:
  explicit PluginUi(Plugin&);

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
  void HandleEvent(ui_notification_t code, va_list args) override;

  Plugin& plugin_;
  UiData data_;
  ShellState state_{ShellState::NO_DB};

  QLabel *time_label_;
  QTimer* timer_;

  forms::StatusWidget* status_form_;
  forms::AddressBookView* address_view_;

  QAction* cnAct_ = nullptr;
  QAction* stAct_ = nullptr;
};