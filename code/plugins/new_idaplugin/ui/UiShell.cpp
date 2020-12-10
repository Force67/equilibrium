// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "UiShell.h"
#include "Plugin.h"

#include <QApplication>

#include <QWindow>
#include <QLabel>
#include <QTimer>
#include <QMenuBar>
#include <QStatusBar>
#include <QMainWindow>

QT::QWidget *GetTopWidget()
{
  return qobject_cast<QMainWindow *>(
      QApplication::activeWindow()->topLevelWidget());
}

QT::QMainWindow *GetMainWindow()
{
  static QMainWindow *s_mainWindow{ nullptr };
  if(!s_mainWindow) {
	for(auto *it : QApplication::topLevelWidgets()) {
	  if(auto *window = qobject_cast<QMainWindow *>(it)) {
		s_mainWindow = window;
		break;
	  }
	}
  }

  return s_mainWindow;
}

namespace {

  // well it is definitely a waste of time, but a good waste of time x)
  constexpr char kWhyAreYouWastingYourTimeText[] = "Time wasted reversing: %1:%2:%3";
} // namespace

UiShell::UiShell(Plugin &plugin) :
    _plugin(plugin)
{
  QMainWindow *window = GetMainWindow();
  assert(window != nullptr);

  hook_to_notification_point(hook_type_t::HT_UI, StaticEvent, this);

  // register a new menu bar for the sync stuff
  QMenu *syncMenu = window->menuBar()->addMenu("RESync");
  syncMenu->addAction("Connect", this, &UiShell::Connect);
  syncMenu->addSeparator();
  //syncMenu->addAction("Settings", this, &UiShell)

  //syncMenu->addAction(QIcon(":/cog"), "Settings", this, &UiView::OpenSettings);

  _wastedTime.reset(new QLabel(""));
  _wastedTime->hide();

  _timer.reset(new QTimer());

  // register tray widgets
  window->statusBar()->addPermanentWidget(_wastedTime.data());

  // and connect everything
  connect(_timer.data(), &QTimer::timeout, this, &UiShell::Tick);
}

UiShell::~UiShell()
{
  unhook_from_notification_point(hook_type_t::HT_UI, StaticEvent, this);
}

void UiShell::HandleEvent(int code, va_list)
{
  switch(code) {
  case ui_notification_t::ui_term:

	break;
  case ui_notification_t::ui_saving: {
	// flush timer
	_store.SetTick(_tick);
	_timer->stop();

	_store.Save();
	break;
  }
  case ui_notification_t::ui_database_inited: {
	_store.Load();

	// initialize tick
	_tick = _store.GetTick();
	_wastedTime->show();
	_timer->start(1000);
	break;
  }
  }
}

void UiShell::Tick()
{
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

void UiShell::Connect()
{
  _plugin.CreateSyncSession();
}

// static handler for ida
ssize_t UiShell::StaticEvent(void *userp, int code, va_list args)
{
  if(auto *self = reinterpret_cast<UiShell *>(userp))
	self->HandleEvent(code, args);

  return 0;
}