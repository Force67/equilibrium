// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/detached_queue.h>

class IdaSync;

// so the compiler doesn't bitch around
struct RequestRunner final : exec_request_t {
 public:
  explicit RequestRunner(IdaSync&);
  ~RequestRunner() override;

  void Queue(const uint8_t* data, size_t size);

  struct Packet;
 private:
  int execute(void) override;

 private:
  base::detached_mpsc_queue<Packet> queue_;
  size_t queueSize_ = 0;
  IdaSync& sync_;
};