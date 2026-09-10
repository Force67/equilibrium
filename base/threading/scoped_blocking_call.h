// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <base/export.h>

namespace base {

// A "blocking call" is any call that waits off-CPU: synchronous file I/O,
// pipes, sockets, file enumeration, and so on. Acquiring a low contention lock
// does not count.

// BlockingType indicates the likelihood that a blocking call will actually
// block.
enum class BlockingType {
  MAY_BLOCK,  // The call might block (e.g. file I/O that may hit a cache).
  WILL_BLOCK  // The call will definitely block.
};

// Annotates a scope that may/will block for the scheduler. Instantiation
// asserts that blocking is allowed via base::AssertBlockingAllowed(). Keep CPU
// usage minimal inside the scope. APIs that block instantiate their own
// ScopedBlockingCall, so do not nest another one around them. Nesting is
// supported; WILL_BLOCK inside MAY_BLOCK wins.
//
// Good:
//   {
//     ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::WILL_BLOCK);
//     data = GetDataFromNetwork();
//   }
//   CPUIntensiveProcessing(data);
//
// Bad:
//   ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::WILL_BLOCK);
//   Data data = GetDataFromNetwork();
//   CPUIntensiveProcessing(data);  // CPU work inside the blocking scope.
//
// From a thread pool task, instantiation grows the pool to compensate for the
// blocked thread (aggressiveness depends on BlockingType).

struct Location {};
#define FROM_HERE \
  Location {}

class BASE_EXPORT ScopedBlockingCall {
 public:
  ScopedBlockingCall(const Location& from_here, BlockingType blocking_type) {}
  ~ScopedBlockingCall() {}
};
}  // namespace base
