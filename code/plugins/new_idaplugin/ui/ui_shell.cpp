// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UiShell.h"
#include "Pch.h"
#include "Plugin.h"

#include <QApplication>

#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QWindow>

#include "features/FeatureDispatch.h"
#include "forms/RunDialog.h"
#include "forms/Settings.h"
#include "forms/StatusWidget.h"

#include "utils/Logger.h"

QT::QWidget* GetTopWidget() {
  return qobject_cast<QMainWindow*>(
      QApplication::activeWindow()->topLevelWidget());
}

QT::QMainWindow* GetMainWindow() {
  static QMainWindow* s_mainWindow{nullptr};
  if (!s_mainWindow) {
    for (auto* it : QApplication::topLevelWidgets()) {
      if (auto* window = qobject_cast<QMainWindow*>(it)) {
        s_mainWindow = window;
        break;
      }
    }
  }

  return s_mainWindow;
}

namespace {

// well it is definitely a waste of time, but a good waste of time x)
constexpr char kWhyAreYouWastingYourTimeText[] =
    "Time wasted reversing: %1:%2:%3";
}  // namespace

// global plugin description
extern plugin_t PLUGIN;

UiShell::UiShell(Plugin& plugin) : _plugin(plugin) {
  QMainWindow* window = GetMainWindow();
  assert(window != nullptr);

  hook_to_notification_point(hook_type_t::HT_UI, StaticEvent, this);

  // register a new menu bar for the sync stuff
  QMenu* syncMenu = window->menuBar()->addMenu("RESync");
  _cnAct = syncMenu->addAction("Connect");
  syncMenu->addSeparator();
  _stAct = syncMenu->addAction(QIcon(":/cog"), "Settings");

  _wastedTime.reset(new QLabel(""));
  _statusForm.reset(new forms::StatusWidget(window->statusBar(), plugin));

  _wastedTime->hide();
  _cnAct->setEnabled(false);

  _timer.reset(new QTimer());

  // register tray widgets
  window->statusBar()->addPermanentWidget(_wastedTime.data());
  window->statusBar()->addPermanentWidget(_statusForm.data());

  // and connect everything
  connect(_timer.data(), &QTimer::timeout, this, &UiShell::Tick);
  connect(_cnAct, &QAction::triggered, &_plugin, &Plugin::ToggleNet);

  connect(_stAct, &QAction::triggered, this, [&]() {
    forms::Settings dia(plugin.client().Connected(), GetTopWidget());
    dia.exec();
  });

  connect(this, &UiShell::ShellStateChange, this, [&](ShellState newState) {
    if (newState == ShellState::IN_DB)
      _cnAct->setEnabled(true);

    if (newState == ShellState::NO_DB)
      _cnAct->setEnabled(false);
  });

  connect(
      &_plugin.session(), &SyncSession::TransportStateChange, this,
      [&](SyncSession::TransportState newState) {
        if (newState == SyncSession::TransportState::ACTIVE)
          _cnAct->setText("Disconnect");

        if (newState == SyncSession::TransportState::DISABLED)
          _cnAct->setText("Connect");
      },
      Qt::QueuedConnection);
}

UiShell::~UiShell() {
  unhook_from_notification_point(hook_type_t::HT_UI, StaticEvent, this);
}

void UiShell::HandleEvent(int code, va_list args) {
  switch (code) {
    case ui_notification_t::ui_database_closed:
      SetShellState(ShellState::NO_DB);

      _timer->stop();
      break;
    case ui_notification_t::ui_saving: {
      // flush timer
      _store.SetTick(_tick);
      _store.Save();
      break;
    }
    case ui_notification_t::ui_database_inited: {
      _store.Load();

      // initialize tick
      _tick = _store.GetTick();
      _wastedTime->show();
      _timer->start(1000);

      SetShellState(ShellState::IN_DB);
      break;
    }
    case ui_notification_t::ui_plugin_unloading: {
      const plugin_info_t* info = va_arg(args, plugin_info_t*);
      if (info && info->entry) {
        if (info->entry == &PLUGIN) {
          ClenseTheShell();
        }
      }
      break;
    }
  }
}

void UiShell::SetShellState(ShellState newState) {
  if (newState != _state) {
    _state = newState;

    emit ShellStateChange(newState);
  }
}

UiShell::ShellState UiShell::GetShellState() const {
  return _state;
}

void UiShell::ClenseTheShell() {
  // manually kill the statusform here, in order to prevent QT from freeing it,
  // which would result in a crash and leave the idb broken...
  _statusForm.reset();
}

void UiShell::Tick() {
  int hours = (_tick / 3600) % 24;
  int minutes = (_tick / 60) % 60;
  int seconds = _tick % 60;

  // format time with leading zeros
  QString text = tr(kWhyAreYouWastingYourTimeText)
                     .arg(hours, 2, 10, QLatin1Char('0'))
                     .arg(minutes, 2, 10, QLatin1Char('0'))
                     .arg(seconds, 2, 10, QLatin1Char('0'));

  _wastedTime->setText(text);
  _tick++;
}

void UiShell::RunFeature() {
  if (_state == ShellState::NO_DB) {
    LOG_ERROR("Open an IDB to run features");
    return;
  }

  features::FeatureIndex index;
  {
    forms::RunDialog dia(GetTopWidget());
    dia.exec();

    index = dia.SelectedIndex();
  }

  if (index != features::FeatureIndex::None) {
    features::DispatchFeatureIndex(index);
  }
}

// static handler for ida
ssize_t UiShell::StaticEvent(void* userp, int code, va_list args) {
  if (auto* self = reinterpret_cast<UiShell*>(userp))
    self->HandleEvent(code, args);

  return 0;
}