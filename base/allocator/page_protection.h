// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/enum_traits.h>

namespace base {
enum class PageProtectionFlags : u32 {
  NONE = 0x0,  // No Access

  R = 0x1,  // Read
  W = 0x2,  // Write
  X = 0x4,  // Execute
  C = 0x8,  // Copy On Write

  G = 0x10,  // Guard

  INVALID = 0x80000000,  // Invalid

  RW = R | W,
  RX = R | X,
  RWX = R | W | X,
  RWC = R | W | C,
  RWXC = R | W | X | C,
};
BASE_IMPL_ENUM_BIT_TRAITS(PageProtectionFlags, u32)

#if defined(OS_WIN)
using NativePageProtectionType = u32;
#elif defined(OS_POSIX)
using NativePageProtectionType = int;  // whatever they define as int.
#endif

NativePageProtectionType TranslateToNativePageProtection(PageProtectionFlags);
PageProtectionFlags TranslateFromNativePageProtection(NativePageProtectionType);
}  // namespace base