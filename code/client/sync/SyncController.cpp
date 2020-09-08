// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "SyncController.h"
#include "SyncClient.h"

SyncController::SyncController(SyncClient &client_) :
	_client(client_) {
  hook_to_notification_point(hook_type_t::HT_IDB, OnIdaEvent, this);
  hook_to_notification_point(hook_type_t::HT_IDP, OnIdaEvent, this);
}

SyncController::~SyncController() {
  unhook_from_notification_point(hook_type_t::HT_IDB, OnIdaEvent, this);
  unhook_from_notification_point(hook_type_t::HT_IDP, OnIdaEvent, this);
}

ssize_t SyncController::OnIdaEvent(void *userp, int code, va_list args) {
  SyncController *self = reinterpret_cast<SyncController *>(userp);
  if (self->_client.IsConnected()) {
	  // check who can handle the emssage
  }


  return 0;
}