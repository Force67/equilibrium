// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "UserInfo.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <qregularexpression.h>

namespace noda {

  const QString &GetDefaultUserName()
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

  const QString &GetUserGuid()
  {
	static QString s_Hwid{};
	if(s_Hwid.isEmpty()) {
#ifdef _WIN32
	  HW_PROFILE_INFOW hwProfileInfo;
	  if(!GetCurrentHwProfileW(&hwProfileInfo))
		return s_Hwid;

	  s_Hwid = QString::fromWCharArray(hwProfileInfo.szHwProfileGuid);
	  s_Hwid.remove(QRegularExpression("[{}]"));
#endif
	}

	return s_Hwid;
  }
} // namespace noda