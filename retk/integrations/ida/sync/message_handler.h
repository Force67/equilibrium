// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "sync/ida_sync.h"
#include <sync/protocol/generated/ida_sync_generated.h>
#include <sync/protocol/generated/message_root_generated.h>

namespace sync {
using namespace protocol::sync;

struct StaticHandler;

struct TaskHandlerRegistry {
  TaskHandlerRegistry() = default;
  ~TaskHandlerRegistry() = default;

  explicit TaskHandlerRegistry(StaticHandler* handler)
      : next(ROOT()), item(handler) {
    ROOT() = this;
  }

  TaskHandlerRegistry* next = nullptr;
  StaticHandler* item = nullptr;

  static TaskHandlerRegistry*& ROOT() {
    static TaskHandlerRegistry* root{nullptr};
    return root;
  }
};

struct StaticHandler final : TaskHandlerRegistry {
  // IDA Event
  hook_type_t hookType = hook_type_t::HT_LAST;
  int hookEvent = -1;

  // Networking
  protocol::MsgType msgType = protocol::MsgType_NONE;

  using apply_t = bool (*)(MsgContext&, const void*);
  using react_t = bool (*)(MsgContext&, va_list);

  // Delegate
  struct BaseHandler {
    apply_t apply;
    react_t react;
  };

  BaseHandler delegates;

  template <typename T>
  struct Handlers : BaseHandler {
    using apply_t = bool (*)(MsgContext&, const T&);

    explicit Handlers(apply_t apply_impl, react_t react_impl) {
      apply = reinterpret_cast<StaticHandler::apply_t>(
          reinterpret_cast<void*>(apply_impl));
      react = react_impl;
    }
  };

  explicit StaticHandler(hook_type_t type,
                         int evt,
                         protocol::MsgType msg,
                         BaseHandler&& methods)
      : hookType(type),
        hookEvent(evt),
        msgType(msg),
        delegates(std::forward<BaseHandler>(methods)),
        TaskHandlerRegistry(this) {}

  ~StaticHandler() = default;
};
}  // namespace sync