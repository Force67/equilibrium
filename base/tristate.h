// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

enum class TriState : i8 { kUnknown = -1, kFalse = 0, kTrue = 1 };

inline bool TriFailed(TriState s) {
  return s == TriState::kFalse || s == TriState::kUnknown;
}

// no custom bool operators here, since they would defeat the very purpose of
// this type
}  // namespace base