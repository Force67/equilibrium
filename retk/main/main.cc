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

static void AssertHandler(const char*, const char*, const char*) {
#if defined(OS_WIN)
  MessageBoxW(nullptr, L"An assertion failed. quitting.", L"RETK", MB_ICONSTOP);
#endif
}

#include <ui/platform/win/native_window_win32.h>
#include <ui/platform/win/message_pump_win.h>

#include <base/filesystem/file_util.h>
#include <program_loader/file_classifier.h>
#include <program_loader/loader_factory.h>

void DOLoadFile(const char *name) {
  i64 size = 0;
  auto contents{base::LoadFile(name, &size)};

  base::Span view{contents.get(), static_cast<size_t>(size)};

  program_loader::FileClassificationInfo info;
  program_loader::ClassifyFile(view, info);

  program_loader::LoaderFactory fa;

  std::vector<const program_loader::ProgramLoadDescriptor*> canidates;
  fa.FindApplicableCanidates(info, canidates);

  std::unique_ptr<program_loader::ProgramLoader> loader;
  for (auto* c : canidates) {
    loader = fa.CreateLoader(*c, info);
  }

  program_loader::ProgramData data;
  if (loader)
    loader->Parse(view, info, data);
}

int main() {
  base::SetCurrentThreadName("AppMain");
  base::SetLogHandler(TKLogHandler);
#if !defined(CONFIG_DEBUG)
  base::SetAssertHandler(AssertHandler);
#endif

#if 0
  ui::NativeWindowWin32 win(u8"RETK");
  win.Init(nullptr, {0,0, 1920, 1080});
  win.Show();

  ui::MessagePumpWin loop;
  while (loop.Update()) {
    loop.Pump();
  }
#endif

  //DOLoadFile(R"(S:\Work\Tilted\TiltedEvolution\build\windows\x64\debug\SkyrimTogether.exe)");
  DOLoadFile(R"(S:\Work\Research\fuchsia\third_party\go\src\debug\dwarf\testdata\line-gcc.elf)");
  // Application app;
  // return app.Exec();


  return 0;
}