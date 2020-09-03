// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiController.h"
#include "sync/SyncClient.h"

#include "forms/UiStatusBar.h"
#include "forms/UiConnectPrompt.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qerrormessage.h>

UiController::UiController(SyncClient& client) : _client(client) {
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
   if (auto *pMenu = pIdaWindow->menuBar()->addMenu(QIcon(":/logo"), "NODA")) {
     pMenu->addAction("Connect", this, &UiController::DoConnect);
     pMenu->addAction("Synchronus", this, &UiController::OpenSyncMenu);
     pMenu->addSeparator();
     pMenu->addAction("Configure", this, &UiController::OnConfigureAct);
   }
}

void UiController::DoConnect() { 
    auto *pIdaWindow = qobject_cast<QMainWindow *>(
      QApplication::activeWindow()->topLevelWidget());

    bool result = _client.Connect(); 
    if (!result) {
      QErrorMessage error(pIdaWindow);
      error.showMessage(
          "Unable to connect to the NODA sync host.\n"
          "It is likely that the selected port is not available.");
      error.exec();
    }
}

void UiController::OpenSyncMenu() {
    msg("OpenSyncMenu");
}

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