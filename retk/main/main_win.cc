// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// the earliest entry point of our application.

#include <Windows.h>

extern int ReTKMain();

#if 0
#define _VCRT_BUILD
#define _VCRT_ALLOW_INTERNALS

#define _SCRT_STARTUP_WWINMAIN
#include <exe_common.inl>

#ifdef CONFIG_DEBUG
#pragma comment(lib, "libcmtd.lib")
#pragma comment(lib, "libvcruntimed.lib")
#pragma comment(lib, "libucrtd.lib")
#else
#pragma comment(lib, "libcmt.lib")
#pragma comment(lib, "libvcruntime.lib")
#pragma comment(lib, "libucrt.lib")
#endif

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

int __stdcall TKWinMain() {
  __security_init_cookie();

  return __scrt_common_main_seh();
}
#endif

struct ComScope {
  bool ok = false;

  ComScope() { ok = SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)); }
  ~ComScope() {
    if (ok)
      ::CoUninitialize();
  }
};

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine,
                      int nCmdShow) {
  ComScope _;
  (void)_;
  return ReTKMain();
}