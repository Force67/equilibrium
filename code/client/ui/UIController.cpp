// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiController.h"

#include "forms/UiStatusBar.h"
#include "forms/UiConnectPrompt.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qobject.h>

UiController::UiController() {
  hook_to_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
}

UiController::~UiController() {
  unhook_from_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
}

void UiController::BuildUi() { 
   auto *pIdaWindow = qobject_cast<QMainWindow *>(
      QApplication::activeWindow()->topLevelWidget());
 
   // pin bottom status bar (online/offline indicator)
   _statusBar.reset(new UiStatusBar());
   _statusBar->SetColor("red");
   _statusBar->show();
   pIdaWindow->statusBar()->addPermanentWidget(_statusBar.data());

   // create the top level menu entry
   if (auto *pMenu = pIdaWindow->menuBar()->addMenu("NODA")) {
     pMenu->addAction("Connect", this, &UiController::OnConnectionAct);
     pMenu->addSeparator();
     pMenu->addAction("Configure", this, &UiController::OnConfigureAct);
   }
}

void UiController::OnConnectionAct() { msg("lol"); }
void UiController::OnConfigureAct() {}

ssize_t UiController::OnUiEvent(void *pUserp, int notificationCode,
                                va_list va) {
  auto *pThis = reinterpret_cast<UiController *>(pUserp);

  if (notificationCode == ui_notification_t::ui_ready_to_run) {
    if (!pThis->_statusBar) {
      pThis->BuildUi();

      UiConnectPromt promt;
      promt.exec();
    }
  }

  return 0;
}