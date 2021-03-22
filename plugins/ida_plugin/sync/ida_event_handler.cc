// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync.h"
#include "ida_event_handler.h"
#include "utils/logger.h"

#include "message_handler.h"

IDAEventHandler::IDAEventHandler(IdaSync& cl) : sync_(cl) {
  hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  hook_to_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
}

IDAEventHandler::~IDAEventHandler() {
  unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  unhook_from_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
}

void IDAEventHandler::HandleEvent(hook_type_t type, int code, va_list args) {
  auto& events = sync_.IdaEvents();

  const auto it = events.find(std::make_pair(type, code));

  if (it == events.end()) {
    LOG_WARNING("HandleEvent() -> Unknown event! {}", code);
    return;
  }

  const auto& handler = it->second->delegates;
  const bool result = handler.react(*this, args);

  if (!result) {
    LOG_ERROR("HandleEvent() -> Failed to execute handler {}", code);
    return;
  }

  LOG_TRACE("HandleEvent() -> Executed handler {}", code);
  sync_.stats.idaEventNum_++;
}

ssize_t IDAEventHandler::IdbEvent(void* ptr, int code, va_list args) {
  if (auto* handler = reinterpret_cast<IDAEventHandler*>(ptr))
	  handler->HandleEvent(hook_type_t::HT_IDB, code, args);

  return 0;
}

ssize_t IDAEventHandler::IdpEvent(void* ptr, int code, va_list args) {
  if (auto* handler = reinterpret_cast<IDAEventHandler*>(ptr))
    handler->HandleEvent(hook_type_t::HT_IDP, code, args);

  return 0;
}