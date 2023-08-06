// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/vector.h>
#include <base/functional/function.h>

namespace base {
class IOUring {
 public:
  struct Operation {
    base::Function<void(void)> callback;
  };

  IOUring() {}

  bool Create();
  void Destroy();

  void WaitForCompletions();

  bool SubmitReadFile(HANDLE fileHandle,
                      void* buffer,
                      UINT32 size,
                      UINT64 offset,
                      IOCompletionCallback callback);

 private:
  void* ring_handle_;
  base::Vector<Operation> pending_queue_;
};
}  // namespace base