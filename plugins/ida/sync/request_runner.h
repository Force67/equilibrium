// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

// This defines an executer class for IDA threaded functions
#pragma once

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
  size_t queueSize_ = 0;
  IdaSync& sync_;
};