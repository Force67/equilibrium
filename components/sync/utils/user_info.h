#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "base/strings/xstring.h"

namespace sync::utils {

#if defined(OS_WIN)
std::string GetSysUserName();

// returns a unique id, based on the users hardware
std::string GetUniqueUserId();
#endif
}