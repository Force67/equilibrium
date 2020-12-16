// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "QuickMenu.h"
#include "Pch.h"
#include "Plugin.h"

#include <QMenu>
#include <QMenuBar>

#include "forms/AboutDialog.h"
#include "forms/Settings.h"

namespace noda {

QuickMenu::QuickMenu(Plugin& plugin, QT::QMenuBar* menuBar) : _plugin(plugin) {
  QMenu* menu = menuBar->addMenu("Noda");

  _connectAct = menu->addAction("Connect", this, &QuickMenu::ToggleConnect);
  //_projectAct = nodaMenu->addAction(QIcon(":/sync"), "Projects", this,
  //&UiController::OpenSyncMenu);
  menu->addSeparator();
  menu->addAction(QIcon(":/cog"), "Settings", this, &QuickMenu::OpenSettings);
  menu->addSeparator();
  menu->addAction(QIcon(":/info"), "About NODA", this,
                  &QuickMenu::OpenAboutDialog);
}

void QuickMenu::OpenAboutDialog() {
  AboutDialog dialog(UiController::GetTopWidget());
  dialog.exec();
}

void QuickMenu::OpenSettings() {
  Settings settings(_plugin.client().Connected(), UiController::GetTopWidget());
  settings.exec();
}

void QuickMenu::ToggleConnect() {}

}  // namespace noda