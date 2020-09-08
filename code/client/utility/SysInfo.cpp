// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "SysInfo.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace utility {

  const QString &GetSysUsername() {
	static QString s_Name;
	if (s_Name.isEmpty()) {
	  char username[260]{};
#ifdef _WIN32
	  DWORD username_len = 260;
	  GetUserNameA(username, &username_len);
#endif

	  s_Name = username;
	}

	return s_Name;
  }

  const QString &GetHardwareId() {
	static QString s_Hwid;
	if (s_Hwid.isEmpty()) {
#ifdef _WIN32
	  HW_PROFILE_INFOA hwProfileInfo;
	  if (! GetCurrentHwProfileA(&hwProfileInfo))
		return {};

	  s_Hwid = hwProfileInfo.szHwProfileGuid;
#endif
	}

	return s_Hwid;
  }
} // namespace utility