// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "application.h"

// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

#include <base/check.h>
#include <base/thread.h>
#include <base/text/code_convert.h>

static void TKLogHandler(base::LogLevel level, const char* msg) {
#if defined(OS_WIN) || defined(CONFIG_DEBUG)
  {
    // const char* src, size_t src_len, base::StringW* output
    base::StringW wide_text;
    BUGCHECK(base::UTF8ToWide(msg, std::strlen(msg), &wide_text));
    OutputDebugStringW(wide_text.c_str());
  }
#endif
}

static void AssertHandler() {
#if defined(OS_WIN)
  MessageBoxW(nullptr, L"An assertion failed. quitting.", L"RETK", MB_ICONSTOP);
#endif
}

#include <ui/platform/win/native_window_win.h>
#include <ui/platform/win/message_pump_win.h>

int main() {
  base::SetCurrentThreadName("AppMain");
  base::SetLogHandler(TKLogHandler);
#if !defined(CONFIG_DEBUG)
  base::SetAssertHandler(AssertHandler);
#endif

  ui::WindowWin win("RETK");
  win.Init(nullptr, {0,0, 1920, 1080});
  win.Show();

  ui::MessagePumpWin loop;
  while (loop.Update()) {
    loop.Pump();
  }

  //Application app;
  //return app.Exec();

  return 0;
}