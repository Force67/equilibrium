// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Defines the global CHECK utility macros.
#pragma once

#include "base/logging.h"

#define TK_UNREACHABLE(expr) (void)expr

// Google style legacy compatability for older code;
// prefer TK_BUGCHECK.
#define TK_DCHECK(expression)                                   \
  do {                                                          \
    if (!(expression)) {                                        \
      ::base::PrintLogMessage(                                  \
          ::base::LogLevel::kError,                             \
          __FUNCTION__ " -> assertion failed at " #expression); \
    }                                                           \
                                                                \
  } while (0)

#define TK_BUGCHECK(expression)                                    \
  do {                                                             \
    if (!(expression)) {                                           \
      ::base::PrintLogMessage(                                     \
          ::base::LogLevel::kError,                                \
          "TKBugcheck FAILED at " __FUNCTION__ "-> " #expression); \
    }                                                              \
                                                                   \
  } while (0)

// (Unreachable)
#define TK_IMPOSSIBLE \
  ::base::PrintLogMessage<>(::base::LogLevel::kError, __FUNCTION__ " impossible " \
                                                                 "reached")