// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiController.h"
#include "forms/UiStatusBar.h"

UiController::UiController() {
  hook_to_notification_point(hook_type_t::HT_UI, UiHandler, this);

  _statusBar.reset(new QT::UiStatusBar());
}

UiController::~UiController() {
  unhook_from_notification_point(hook_type_t::HT_UI, UiHandler, this);
}

ssize_t UiController::UiHandler(void *pUserp, int notificationCode, va_list va) {
  if (notificationCode == ui_notification_t::ui_ready_to_run) {

  }

  return 0;
}