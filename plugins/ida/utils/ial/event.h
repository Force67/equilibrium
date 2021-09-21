// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace ial {
template <hook_type_t HT, typename NT>
class EventHandler {
 public:
  EventHandler() { hook_to_notification_point(HT, StaticHandler, this); }
  ~EventHandler() { unhook_from_notification_point(HT, StaticHandler, this); }
  // TODO: CXX 20 constexpr virtual
  // we make overriding multiple inherited event handlers possible
  //    This allows us to do something like this:
  //class... : public UiEventHandler,
  //           public IdbEventHandler {
  // void HandleEvent(ui_notification_t code, va_list args) override {}
  // void HandleEvent(idb_event::event_code_t code, va_list args) override {}
  virtual void HandleEvent(NT code, va_list args) = 0;

 private:
  static inline ssize_t StaticHandler(void* userp, int code, va_list args) {
    if (auto* self = reinterpret_cast<EventHandler<HT, NT>*>(userp))
      self->HandleEvent(static_cast<NT>(code), args);
    return 0;
  }
};
using UiEventHandler = EventHandler<hook_type_t::HT_UI, ui_notification_t>;
using IdbEventHandler =
    EventHandler<hook_type_t::HT_IDB, idb_event::event_code_t>;
}  // namespace ial