#pragma once

// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <string>
#include <vector>

namespace base {

#ifdef _WIN32
std::string utf16_to_utf8(const std::wstring_view input);
std::wstring utf8_to_utf16_w(const std::string& str);
#endif
}