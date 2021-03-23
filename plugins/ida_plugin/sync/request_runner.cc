// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync.h"
#include "request_runner.h"
#include "message_handler.h"
#include "sync/protocol/generated/message_root_generated.h"

#include <base/object_pool.h>

struct RequestRunner::Packet {
  uint32_t dataSize;
  std::unique_ptr<uint8_t[]> data;
  base::detached_queue_key<Packet> key;
};

static base::object_pool<RequestRunner::Packet> s_Pool;

RequestRunner::RequestRunner(IdaSync& cl) : sync_(cl) {

}

RequestRunner::~RequestRunner() {

}

void RequestRunner::Queue(const uint8_t* data, size_t size) {
  auto* task = s_Pool.allocate();
  task->key.next = nullptr;
  task->data = std::make_unique<uint8_t[]>(size);

  std::memcpy(task->data.get(), data, size);
  queue_.push(&task->key);

  queueSize_++;
  if (queueSize_ == 1)
    execute_sync(*this, MFF_WRITE | MFF_NOWAIT);
}

int RequestRunner::execute() {
  auto& events = sync_.NetEvents();

  while (auto* item = queue_.pop(&Packet::key)) {
    queueSize_--;

    // unpack the message
    const protocol::MessageRoot* root =
        protocol::GetMessageRoot(item->data.get());

    int tt = static_cast<int>(root->msg_type());
    auto it = events.find(tt);
    if (it != events.end()) {
      if (it->second->delegates.apply(sync_.Context(), root->msg()))
        sync_.Data().version_++;
    }

    s_Pool.destruct(item);
  }
  
  return 0;
}