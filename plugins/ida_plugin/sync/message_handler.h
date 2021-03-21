// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#pragma once

#include "Pch.h"
#include "sync/sync_service.h"
#include "sync/sync_client.h"
#include "utils/Logger.h"

#include "protocol/generated/IdaSync_generated.h"
#include "protocol/generated/MessageRoot_generated.h"

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

  using apply_t = bool (*)(SyncService&, const void*);
  using react_t = bool (*)(SyncService&, va_list);

  // Delegate
  struct BaseHandler {
    apply_t apply;
    react_t react;
  };

  BaseHandler delegates;

  template <typename T>
  struct Handlers : BaseHandler {
    using apply_t = bool (*)(SyncService&, const T&);

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