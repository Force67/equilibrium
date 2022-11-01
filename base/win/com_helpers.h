// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <Shlwapi.h>

namespace base::win {

struct ComScope {
  bool ok = false;

  ComScope() { ok = SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)); }
  ~ComScope() {
    if (ok)
      ::CoUninitialize();
  }
};
}  // namespace base::win