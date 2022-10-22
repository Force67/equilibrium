// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/compiler.h>
#include <base/meta/source_location.h>

namespace base {
namespace detail {
void InvokeOutOfMemoryHandler(const base::SourceLocation&,
                              const char* reason = nullptr);
}

using OutOfMemoryHandler = void(void*, const char*);
void SetOutOfMemoryHandler(OutOfMemoryHandler*, void* user_context = nullptr);
void SetOutOfMemoryContext(void* user_context);

// this function is used to report memory allocation errors
#define BASE_INVOKE_OOM(...)                                           \
  MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__);                 \
  ::base::detail::InvokeOutOfMemoryHandler(kSourceLoc, ##__VA_ARGS__); \
  CHECK_BREAK;
}  // namespace base