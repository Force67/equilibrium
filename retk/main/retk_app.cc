// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/logging.h>
#include <base/threading/thread.h>
#include <base/allocator/memory_coordinator.h>
#include <base/memory/distinct_pointer_experimental.h>

#include <main/error_handler.h>
#include <main/log_handler.h>

namespace main {

class ReTKApplication {
 public:
  ReTKApplication() {}
  ~ReTKApplication() {}

  int Exec() {
    LOG_INFO("Starting ReTK");


    return 0;
  }

 private:
  // this _must_ come first.
  main::LogHandler log_handler_;
};
}  // namespace main

int ReTKMain() {
  {
    main::InstallErrorHandlers();
    // NOTE(Vince): this could assert depending on the platform, so set the name last
    base::SetCurrentThreadName("Main");
  }

  base::DistinctPointer<main::ReTKApplication> app;
  return app->Exec();
}