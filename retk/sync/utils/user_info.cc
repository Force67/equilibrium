// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "user_info.h"
#include "base/string/code_convert.h"

#if defined(OS_WIN)
#include <Windows.h>
#endif

namespace sync::utils {

#if defined(OS_WIN)
std::string GetSysUserName() {

  return {};
}

std::string GetUniqueUserId() {
  HW_PROFILE_INFOW hwProfileInfo;
  if (!GetCurrentHwProfileW(&hwProfileInfo))
    return {};

  return {};
  //return base::utf16_to_utf8(hwProfileInfo.szHwProfileGuid);
}
#endif
}