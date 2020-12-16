// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Pch.h"

#include <map>
#include "protocol/generated/MessageRoot_generated.h"

#include "utility/DetachedQueue.h"
#include "utility/ObjectPool.h"

namespace noda {
class SyncController;

struct TaskHandler;

struct TaskDispatcher : exec_request_t {
  explicit TaskDispatcher(SyncController&);

  void QueueTask(const uint8_t* data, size_t size);
  void DispatchEvent(hook_type_t t, int, va_list);

 private:
  int idaapi execute() override;

  struct Task {
    std::unique_ptr<uint8_t[]> data;
    utility::detached_queue_key<Task> key;
  };

  utility::detached_mpsc_queue<Task> _taskQueue;
  utility::object_pool<Task> _taskPool;
  std::atomic_int _eventSize = 0;

  SyncController& _sc;

  using IdaEventType_t = std::pair<hook_type_t, int>;
  std::map<IdaEventType_t, TaskHandler*> _idaEvents;
  std::map<protocol::MsgType, TaskHandler*> _netEvents;
};
}  // namespace noda