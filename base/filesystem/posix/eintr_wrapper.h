// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Wrapper around system calls that may be interrupted by a signal and return
// EINTR (see man 7 signal). In Debug builds a retry limit makes persistent
// EINTR loops (a likely bug) surface instead of spinning forever.
//
// A no-op on Windows and Fuchsia (no signals).
//
// Do not wrap close() in HANDLE_EINTR; use IGNORE_EINTR if the return value
// matters.

#pragma once

#include "build/build_config.h"

#if defined(OS_POSIX)

#include <errno.h>

// An immediately-invoked lambda rather than a GNU braced-group (`({ ... })`):
// the braced-group is an extension that ISO C++ forbids inside an expression,
// and the lambda gives the same expression semantics portably. The reference
// capture is what lets |x| still name the caller's locals.

#if defined(NDEBUG)

#define HANDLE_EINTR(x)                                       \
  ([&]() -> decltype(x) {                                     \
    decltype(x) eintr_wrapper_result;                         \
    do {                                                      \
      eintr_wrapper_result = (x);                             \
    } while (eintr_wrapper_result == -1 && errno == EINTR);   \
    return eintr_wrapper_result;                              \
  }())

#else

#define HANDLE_EINTR(x)                                        \
  ([&]() -> decltype(x) {                                      \
    int eintr_wrapper_counter = 0;                             \
    decltype(x) eintr_wrapper_result;                          \
    do {                                                       \
      eintr_wrapper_result = (x);                              \
    } while (eintr_wrapper_result == -1 && errno == EINTR &&   \
             eintr_wrapper_counter++ < 100);                   \
    return eintr_wrapper_result;                               \
  }())

#endif  // NDEBUG

#define IGNORE_EINTR(x)                                     \
  ([&]() -> decltype(x) {                                   \
    decltype(x) eintr_wrapper_result = (x);                 \
    if (eintr_wrapper_result == -1 && errno == EINTR) {     \
      eintr_wrapper_result = 0;                             \
    }                                                       \
    return eintr_wrapper_result;                            \
  }())

#else  // !BUILDFLAG(IS_POSIX)

#define HANDLE_EINTR(x) (x)
#define IGNORE_EINTR(x) (x)

#endif  // !BUILDFLAG(IS_POSIX)
