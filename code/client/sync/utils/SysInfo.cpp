// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#include "SysInfo.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace noda::sync::utils
{
  const QString &GetSysUsername()
  {
	static QString s_Name{};
	if(s_Name.isEmpty()) {
	  wchar_t username[260]{};
#ifdef _WIN32
	  DWORD username_len = 260;
	  GetUserNameW(username, &username_len);
#endif

	  s_Name = QString::fromWCharArray(username);
	}

	return s_Name;
  }

  const QString &GetHardwareId()
  {
	static QString s_Hwid{};
	if(s_Hwid.isEmpty()) {
#ifdef _WIN32
	  HW_PROFILE_INFOA hwProfileInfo;
	  if(!GetCurrentHwProfileA(&hwProfileInfo))
		return s_Hwid;

	  s_Hwid = hwProfileInfo.szHwProfileGuid;
#endif
	}

	return s_Hwid;
  }
} // namespace utility