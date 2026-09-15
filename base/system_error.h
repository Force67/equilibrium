// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Naming a platform error code, without strerror.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

// The symbolic name of |error| ("ENOENT", "EACCES"), or "E<number>" for a
// code this does not know.
//
// Deliberately not strerror's prose. That is translated, so the same failure
// reads differently depending on the machine's locale, which is the opposite
// of what a log wants: the symbolic name is greppable, stable across hosts,
// and is what the manual pages and the caller's own code are written in.
// The returned pointer is a string literal and outlives any caller.
BASE_EXPORT const char* ErrnoName(int error) noexcept;

}  // namespace base
