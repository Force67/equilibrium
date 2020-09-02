// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiController.h"
#include "forms/UiStatusBar.h"

#include <qmainwindow.h>
#include <qstatusbar.h>

UiController::UiController() {
  hook_to_notification_point(hook_type_t::HT_UI, UiHandler, this);
}

UiController::~UiController() {
  unhook_from_notification_point(hook_type_t::HT_UI, UiHandler, this);
}

ssize_t UiController::UiHandler(void *pUserp, int notificationCode, va_list va) {
  if (notificationCode == ui_notification_t::ui_ready_to_run) {
    auto& _statusBar = reinterpret_cast<UiController*>(pUserp)->_statusBar;
    if (!_statusBar) {
        auto *mainWindow = qobject_cast<QMainWindow *>(
              QApplication::activeWindow()->topLevelWidget());

      if (mainWindow) {

        auto *statusBar = mainWindow->statusBar();
        if (statusBar) {
          _statusBar.reset(new QT::UiStatusBar());
          _statusBar->show();

          statusBar->addPermanentWidget(_statusBar.data());
          return 0;
        }
      
      }
    }

    _statusBar->SetColor("red");
  }

  return 0;
}