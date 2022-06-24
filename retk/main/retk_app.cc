// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/logging.h>
#include <base/threading/thread.h>
#include <base/allocator/memory_coordinator.h>
#include <base/memory/distinct_pointer_experimental.h>

namespace {

class ReTKApplication {
 public:
  ReTKApplication() {}
  ~ReTKApplication() {}

  int Exec() { return 0; }

  friend void HandleOOM(void*, base::MemoryCoordinator&);
  friend void HandleAssertion(const char*, const char*, const char*, const char*);
  friend void HandleLogMessage(void*, base::LogLevel, const char*);

 private:
};

void HandleOOM(void* user_pointer, base::MemoryCoordinator&) {}

void HandleAssertion(const char*, const char*, const char*, const char*) {}

void HandleLogMessage(void*, base::LogLevel, const char*) {}
}  // namespace

int ReTKMain() {
  // create the core context to respond to events before the app exists
  {
    base::SetOutOfMemoryHandler(HandleOOM, nullptr);
    base::SetLogHandler(HandleLogMessage, nullptr);
    base::SetAssertHandler(HandleAssertion);
    // NOTE(Vince): this could assert depending on the platform, so set the name last
    base::SetCurrentThreadName("Main");
  }

  base::DistinctPointer<ReTKApplication> app;
  return app->Exec();
}