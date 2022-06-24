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

  static void OnOutOfMemory(void* user_pointer, base::MemoryCoordinator& mc) {}

  static void HandleAssertion(const char*, const char*, const char*, const char*) {
    // messagebox?
  }

  static void OnLogMessage(void* user_pointer,
                           base::LogLevel level,
                           const char* msg) {}

  int Exec() { return 0;
  }

 private:
};
using AppPointer = base::DistinctPointer<ReTKApplication>;

struct AppFacade {
  
};
}  // namespace

int ReTKMain() {
  // create the core context to respond to events before the app exists
  AppFacade facade;
  {
    base::SetOutOfMemoryHandler(ReTKApplication::OnOutOfMemory, &facade);
    base::SetLogHandler(ReTKApplication::OnLogMessage, &facade);
    base::SetAssertHandler(ReTKApplication::HandleAssertion);
    // NOTE(Vince): this could assert depending on the platform, so set the name last
    base::SetCurrentThreadName("Main");
  }

  AppPointer app;
  return app->Exec();
}