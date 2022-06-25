// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/export.h>
#include <base/check.h>
#include <base/logging.h>

namespace base {

namespace {
// by default we forward those cases to the global log handler
static void DefaultCheckHandler(const char* message,
                                 const char* file_name,
                                 const char* function,
                                 const char* msg) {
  ::base::PrintLogMessage(::base::LogLevel::kFatal, message, file_name, function);
  if (msg)
    ::base::PrintLogMessage(::base::LogLevel::kFatal, msg);
}

BASE_EXPORT constinit base::CheckHandler* assert_handler{DefaultCheckHandler};
}  // namespace

namespace detail {
void DCheck(const SourceLocation& source_location, const char* msg) {
  assert_handler(source_location.format, source_location.file, source_location.func,
                 msg);
}

void BugCheck(const SourceLocation& source_location, const char* msg) {
  assert_handler(source_location.format, source_location.file, source_location.func,
                 msg);
}
}  // namespace detail

void SetCheckHandler(CheckHandler handler) {
  assert_handler = handler;
}
}  // namespace base
