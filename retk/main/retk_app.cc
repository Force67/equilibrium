// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/logging.h>
#include <base/profiling.h>
#include <base/threading/thread.h>
#include <base/allocator/memory_coordinator.h>
#include <base/memory/distinct_pointer_experimental.h>

#include <main/error_handler.h>
#include <main/log_handler.h>

#include <main_window.h>
#include <ui/platform/win/message_pump_win.h>

//#define RETK_APP_HEADLESS

namespace main {

class ReTKApplication {
 public:
  ReTKApplication() {}
  ~ReTKApplication() {}

  int Exec();

 private:
  bool Initialize();

 private:
  // this _must_ come first.
  main::LogHandler log_handler_;
#ifndef BASE_IS_HEADLESS
  MainWindow main_window_;
#endif
};

bool ReTKApplication::Initialize() {
  BASE_PROFILE("App::Initialize");
  main_window_.Initialize();
  return true;
}

int ReTKApplication::Exec() {
  LOG_INFO("Starting ReTK");

  if (!Initialize())
    return 0;

  ui::MessagePumpWin message_pump;
  while (message_pump.Update()) {
    BASE_PROFILE_FRAME("Main");
    message_pump.Pump();
  }

  return 0;
}
}  // namespace main

int ReTKMain() {
  {
    main::InstallErrorHandlers();
    // NOTE(Vince): this could assert depending on the platform, so set the name last
    base::SetCurrentThreadName("Main");
  }

  // keep the stack free
  base::DistinctPointer<main::ReTKApplication> app;
  return app->Exec();
}