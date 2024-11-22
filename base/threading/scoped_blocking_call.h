// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <base/export.h>

namespace base {

// A "blocking call" refers to any call that causes the calling thread to wait
// off-CPU. It includes but is not limited to calls that wait on synchronous
// file I/O operations: read or write a file from disk, interact with a pipe or
// a socket, rename or delete a file, enumerate files in a directory, etc.
// Acquiring a low contention lock is not considered a blocking call.

// BlockingType indicates the likelihood that a blocking call will actually
// block.
enum class BlockingType {
  // The call might block (e.g. file I/O that might hit in memory cache).
  MAY_BLOCK,
  // The call will definitely block (e.g. cache already checked and now pinging
  // server synchronously).
  WILL_BLOCK
};

// This class must be instantiated in every scope where a blocking call is made
// and serves as a precise annotation of the scope that may/will block for the
// scheduler. When a ScopedBlockingCall is instantiated, it asserts that
// blocking calls are allowed in its scope with a call to
// base::AssertBlockingAllowed(). CPU usage should be minimal within that scope.
// //base APIs that block instantiate their own ScopedBlockingCall; it is not
// necessary to instantiate another ScopedBlockingCall in the scope where these
// APIs are used. Nested ScopedBlockingCalls are supported (mostly a no-op
// except for WILL_BLOCK nested within MAY_BLOCK which will result in immediate
// WILL_BLOCK semantics).
//
// Good:
//   Data data;
//   {
//     ScopedBlockingCall scoped_blocking_call(
//         FROM_HERE, BlockingType::WILL_BLOCK);
//     data = GetDataFromNetwork();
//   }
//   CPUIntensiveProcessing(data);
//
// Bad:
//   ScopedBlockingCall scoped_blocking_call(FROM_HERE,
//       BlockingType::WILL_BLOCK);
//   Data data = GetDataFromNetwork();
//   CPUIntensiveProcessing(data);  // CPU usage within a ScopedBlockingCall.
//
// Good:
//   Data a;
//   Data b;
//   {
//     ScopedBlockingCall scoped_blocking_call(
//         FROM_HERE, BlockingType::MAY_BLOCK);
//     a = GetDataFromMemoryCacheOrNetwork();
//     b = GetDataFromMemoryCacheOrNetwork();
//   }
//   CPUIntensiveProcessing(a);
//   CPUIntensiveProcessing(b);
//
// Bad:
//   ScopedBlockingCall scoped_blocking_call(
//       FROM_HERE, BlockingType::MAY_BLOCK);
//   Data a = GetDataFromMemoryCacheOrNetwork();
//   Data b = GetDataFromMemoryCacheOrNetwork();
//   CPUIntensiveProcessing(a);  // CPU usage within a ScopedBlockingCall.
//   CPUIntensiveProcessing(b);  // CPU usage within a ScopedBlockingCall.
//
// Good:
//   base::WaitableEvent waitable_event(...);
//   waitable_event.Wait();
//
// Bad:
//  base::WaitableEvent waitable_event(...);
//  ScopedBlockingCall scoped_blocking_call(
//      FROM_HERE, BlockingType::WILL_BLOCK);
//  waitable_event.Wait();  // Wait() instantiates its own ScopedBlockingCall.
//
// When a ScopedBlockingCall is instantiated from a ThreadPool parallel or
// sequenced task, the thread pool size is incremented to compensate for the
// blocked thread (more or less aggressively depending on BlockingType).

struct Location {};
#define FROM_HERE \
  Location {}

class BASE_EXPORT ScopedBlockingCall {
 public:
  ScopedBlockingCall(const Location& from_here, BlockingType blocking_type) {}
  ~ScopedBlockingCall() {}
};
}  // namespace base
