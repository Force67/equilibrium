// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/logging.h>

namespace base {

namespace {
// by default we forward those cases to the global log handler
static void DefaultAssertHandler(const char* message,
                                 const char* file_name,
                                 const char* msg) {
  ::base::PrintLogMessage(::base::LogLevel::kFatal, message, file_name);
}

static base::AssertHandler* assert_handler{DefaultAssertHandler};
}  // namespace

namespace detail {
void DCheck(const SourceLocation& source_location, const char *msg) {
  assert_handler(source_location.text, source_location.file, msg);
}
void BugCheck(const SourceLocation& source_location, const char *msg) {
  assert_handler(source_location.text, source_location.file, msg);
}
}  // namespace detail

void SetAssertHandler(AssertHandler handler) {
  assert_handler = handler;
}
}  // namespace base
