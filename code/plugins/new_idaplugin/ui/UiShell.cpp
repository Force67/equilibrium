// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "UiShell.h"
#include "Plugin.h"

#include <QApplication>

#include <QWindow>
#include <QMenuBar>
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

UiShell::UiShell(Plugin &plugin) :
    _plugin(plugin)
{
  QMainWindow *window = GetMainWindow();
  assert(window != nullptr);

  hook_to_notification_point(hook_type_t::HT_UI, UiEvent, this);

  // register a new menu bar for the sync stuff
  QMenu *syncMenu = window->menuBar()->addMenu("RESync");
  syncMenu->addAction("Connect", this, &UiShell::Connect);
  syncMenu->addSeparator();
  //syncMenu->addAction("Settings", this, &UiShell)

  //syncMenu->addAction(QIcon(":/cog"), "Settings", this, &UiView::OpenSettings);
}

UiShell::~UiShell()
{
  unhook_from_notification_point(hook_type_t::HT_UI, UiEvent, this);
}

void UiShell::Connect()
{
  _plugin.CreateSyncSession();
}

ssize_t UiShell::UiEvent(void *, int code, va_list)
{
  return 0;
}