#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <string>

namespace base {

#if defined(OS_WIN)
// user name
std::string GetSysUserName();

// returns a unique id, based on the users hardware
std::string GetUniqueUserId();
#endif
}