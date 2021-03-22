// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "user_info.h"
#include "base/string_util.h"

#if defined(OS_WIN)
#include <Windows.h>
#endif

namespace sync::utils {

#if defined(OS_WIN)
std::string GetSysUserName() {
  wchar_t username[260]{};
#ifdef _WIN32
  DWORD username_len = 260;
  GetUserNameW(username, &username_len);
#endif

  return base::utf16_to_utf8(username);
}

std::string GetUniqueUserId() {
  HW_PROFILE_INFOW hwProfileInfo;
  if (!GetCurrentHwProfileW(&hwProfileInfo))
    return {};

  return base::utf16_to_utf8(hwProfileInfo.szHwProfileGuid);
}
#endif
}