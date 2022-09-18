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
#include <debug_window.h>

#include <ui/platform/win/message_pump_win.h>

#if defined(ENABLE_PROFILE)
#include <base/filesystem/path.h>
#include <base/process/process.h>
#endif

//#define RETK_APP_HEADLESS

namespace main {

class ReTKApplication {
 public:
  ReTKApplication() {}
  ~ReTKApplication() {}

  int Exec();

 private:
  bool Initialize();
  void Update();

 private:
  // this _must_ come first.
  main::LogHandler log_handler_;
#ifndef RETK_IS_HEADLESS
  MainWindow main_window_;
  DebugWindow debug_window_;
#endif
};

bool ReTKApplication::Initialize() {
  BASE_PROFILE("App::Initialize");
#ifndef RETK_IS_HEADLESS
  main_window_.Initialize();
  debug_window_.Initialize();
#endif
  return true;
}

void ReTKApplication::Update() {

}

int ReTKApplication::Exec() {
  LOG_INFO("Starting ReTK");

  if (!Initialize())
    return 0;

  ui::MessagePumpWin message_loop;
  while (!message_loop.WantsToQuit()) {
    BASE_PROFILE_FRAME("Main");
    if (message_loop.Peek())
      message_loop.Pump();
    else
      Update();
  }

  LOG_INFO("Shutting down ReTK");
  return 0;
}
}  // namespace main

bool AttachProfiler() {
  // hardcode the target address for now.
#if defined(ENABLE_PROFILE)
  // dispatch tracy
  if (!base::SpawnProcess(
          R"(C:\Users\vince\Desktop\DepotTools\Tracy-0.8.2\Tracy.exe)", u8"")) {
    // -a 127.0.0.1 TODO: auto attach
    MessageBoxW(0, L"Failed to start profiler", L"ReTK", MB_ICONSTOP);
    return false;
  }

  LOG_INFO("Launched profiler at {}", "127.0.0.1");
#endif
  return true;
}

int ReTKMain() {
  {
    main::InstallErrorHandlers();
    // NOTE(Vince): this could assert depending on the platform, so set the name last
    base::SetCurrentThreadName("Main");
  }

  if (!AttachProfiler())
    return -1;

  // keep the stack free
  base::DistinctPointer<main::ReTKApplication> app;
  return app->Exec();
}