// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#pragma once

#include "SyncController.h"
#include "protocol/generated/MessageRoot_generated.h"
#include "utils/Logger.h"

namespace noda {
using namespace protocol::sync;

struct TaskHandlerRegistry {
  TaskHandlerRegistry() = default;
  ~TaskHandlerRegistry() = default;

  TaskHandlerRegistry(TaskHandler* hndlr) : next(ROOT()), handler(hndlr) {
    ROOT() = this;
  }

  TaskHandlerRegistry* next = nullptr;
  TaskHandler* handler = nullptr;

  static TaskHandlerRegistry*& ROOT() {
    static TaskHandlerRegistry* root{nullptr};
    return root;
  }
};

struct TaskHandler : TaskHandlerRegistry {
  // IDA Event
  hook_type_t hookType = hook_type_t::HT_LAST;
  int hookEvent = -1;

  // Networking
  protocol::MsgType msgType = protocol::MsgType_NONE;

  using apply_t = bool (*)(SyncController&, const void*);
  using react_t = bool (*)(SyncController&, va_list);

  // Delegate
  struct BaseHandler {
    apply_t apply;
    react_t react;
  };

  BaseHandler delegates;

  template <typename T>
  struct Handlers : BaseHandler {
    using apply_t = bool (*)(SyncController&, const T&);

    explicit Handlers(apply_t apply_impl, react_t react_impl) {
      apply = reinterpret_cast<TaskHandler::apply_t>(
          reinterpret_cast<void*>(apply_impl));
      react = react_impl;
    }
  };

  explicit TaskHandler(hook_type_t type,
                       int evt,
                       protocol::MsgType msg,
                       BaseHandler&& methods)
      : hookType(type),
        hookEvent(evt),
        msgType(msg),
        delegates(std::forward<BaseHandler>(methods)),
        TaskHandlerRegistry(this) {}

  ~TaskHandler() = default;
};
}  // namespace noda