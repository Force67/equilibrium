// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "plugin.h"
#include "plugin_ui.h"

#include <QApplication>

#include <QLabel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QWindow>

#include "forms/run_dialog.h"
#include "forms/settings.h"
#include "forms/status_widget.h"
#include "forms/address_book_view.h"

#include "tools/quick_runner.h"

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

PluginUi::PluginUi(Plugin& plugin) : plugin_(plugin) {
  QMainWindow* window = GetMainWindow();
  assert(window != nullptr);

  hook_to_notification_point(hook_type_t::HT_UI, StaticEvent, this);

  static AddressBookData* s_fakeDataTemp = new AddressBookData();

  time_label_ = new QLabel("");
  status_form_ = new forms::StatusWidget(window->statusBar(), plugin);
  address_view_ = new forms::AddressBookView(*s_fakeDataTemp);

  // register the address book selection view toggle
  QMenu* viewMenu =
      reinterpret_cast<QMenu*>(window->menuBar()->actions()[4]->parent());
  QMenu* subViews = reinterpret_cast<QMenu*>(viewMenu->actions()[0]->parent());
  QAction* before = subViews->actions()[12];

  auto* abAct_ = new QAction(QIcon(":/book"), "Address Book", subViews);
  subViews->insertAction(before, abAct_);

  // register a new menu bar for the sync stuff
  QMenu* syncMenu = window->menuBar()->addMenu("ReTK");
  cnAct_ = syncMenu->addAction("Connect");
  syncMenu->addSeparator();
  stAct_ = syncMenu->addAction(QIcon(":/cog"), "Settings");

  time_label_->hide();
  cnAct_->setEnabled(false);
  timer_ = new QTimer();

  // register tray widgets
  window->statusBar()->addPermanentWidget(time_label_);
  window->statusBar()->addPermanentWidget(status_form_);

  // and connect everything
  connect(timer_, &QTimer::timeout, this, &PluginUi::Tick);
  connect(cnAct_, &QAction::triggered, &plugin_, &Plugin::SyncToggle);

  QObject::connect(abAct_, &QAction::triggered, this,
                   [&]() { address_view_->Show(true); });

  connect(stAct_, &QAction::triggered, this, [&]() {
    // bool isOnline = plugin.Sync().Client().Connected();

    bool isOnline = false;

    forms::Settings dia(isOnline, GetTopWidget());
    dia.exec();
  });

  connect(this, &PluginUi::ShellStateChange, this, [&](ShellState newState) {
    if (newState == ShellState::IN_DB)
      cnAct_->setEnabled(true);

    if (newState == ShellState::NO_DB)
      cnAct_->setEnabled(false);
  });

#if 0
  connect(
      &plugin_.Sync(), &IdaSync::StateChange, this,
      [&](IdaSync::State state) {
        if (state == IdaSync::State::kActive)
          cnAct_->setText("Disconnect");

        if (state == IdaSync::State::kDisabled)
          cnAct_->setText("Connect");
      },
      Qt::QueuedConnection);
#endif
}

PluginUi::~PluginUi() {
  unhook_from_notification_point(hook_type_t::HT_UI, StaticEvent, this);
}

void PluginUi::HandleEvent(int code, va_list args) {
  switch (code) {
    case ui_notification_t::ui_database_closed:
      SetShellState(ShellState::NO_DB);

      timer_->stop();
      break;
    case ui_notification_t::ui_database_inited: {
      time_label_->show();
      timer_->start(1000);
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

  address_view_->HandleNotification(code, args);
}

void PluginUi::SetShellState(ShellState newState) {
  if (newState != state_) {
    state_ = newState;

    emit ShellStateChange(newState);
  }
}

PluginUi::ShellState PluginUi::GetShellState() const {
  return state_;
}

void PluginUi::ClenseTheShell() {
  // manually kill the statusform here, in order to prevent QT from freeing it,
  // which would result in a crash and leave the idb broken...
  //statusForm_.reset();
}

void PluginUi::Tick() {
  int hours = (data_.tick_ / 3600) % 24;
  int minutes = (data_.tick_ / 60) % 60;
  int seconds = data_.tick_ % 60;

  // format time with leading zeros
  QString text = tr(kWhyAreYouWastingYourTimeText)
                     .arg(hours, 2, 10, QLatin1Char('0'))
                     .arg(minutes, 2, 10, QLatin1Char('0'))
                     .arg(seconds, 2, 10, QLatin1Char('0'));

  time_label_->setText(text);
  data_.tick_++;
}

void PluginUi::RunFeature() {
  using RD = forms::RunDialog;

  if (state_ == ShellState::NO_DB) {
    LOG_ERROR("Features are not available on a closed IDB.");
    return;
  }

  RD dialog(GetTopWidget());
  dialog.exec();

  if (dialog.SelectedIndex() == 0)
    return;
  // Due to the way RunFeature() is invoked
  // we are already on a valid IDA thread.
  // Still, we move execution to the new runner component
  // while also hiding index
  emit RequestFeature(dialog.SelectedIndex());
}

// static handler for ida
ssize_t PluginUi::StaticEvent(void* userp, int code, va_list args) {
  if (auto* self = reinterpret_cast<PluginUi*>(userp))
    self->HandleEvent(code, args);

  return 0;
}