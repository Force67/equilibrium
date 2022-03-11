// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/logging.h>

namespace base {

namespace {
// by default we forward those cases to the global log handler
static void DefaultAssertHandler(const char* message, const char* file_name) {
  ::base::PrintLogMessage(::base::LogLevel::kFatal, message, file_name);
}

static base::AssertHandler* assert_handler{DefaultAssertHandler};
}  // namespace

void SetAssertHandler(AssertHandler handler) {
  assert_handler = handler;
}

void InvokeAssertHandler(const char* message, const char* origin_file_name) {
  assert_handler(message, origin_file_name);
}
}  // namespace base
