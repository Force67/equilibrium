// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/export.h>
#include <base/check.h>
#include <base/logging.h>
#include <base/standard_streams.h>
#include <base/strings/format.h>


namespace base {

namespace {
constexpr char kBaseCheckChannelName[] = "CHECK";

static void DefaultCheckHandler(const char* message,
                                const char* /*file_name*/,
                                const char* /*function*/,
                                const char* msg) {
  ::base::detail::WriteLogMessage(kBaseCheckChannelName, ::base::LogLevel::kFatal, message);
  if (msg)
    ::base::detail::WriteLogMessage(kBaseCheckChannelName, ::base::LogLevel::kFatal, msg);
}

BASE_EXPORT constinit base::CheckHandler* assert_handler{DefaultCheckHandler};
}  // namespace

namespace detail {
void DCheck(const SourceLocation& source_location, const char* msg) {
  assert_handler(source_location.format, source_location.file, source_location.func, msg);
}

void BugCheck(const SourceLocation& source_location, const char* msg) {
  assert_handler(source_location.format, source_location.file, source_location.func, msg);
}

void FatalCheckFailure(const char* file, int line, const char* message) {
  // stderr first and unconditionally. The log path runs through a channel
  // filter and an installable callback, either of which can drop the message,
  // and the one thing that must survive a fatal check is the reason for it.
  char reason[640];
  const mem_size length = base::FormatTo(reason, sizeof(reason),
                                         "fatal check: {}:{} {}\n", file, line,
                                         message ? message : "");
  base::WriteStandardError(reason,
                           length < sizeof(reason) ? length : sizeof(reason));

  // The handler and its log sink may allocate, and the most common fatal check
  // is an allocation one, so a second failure while reporting the first would
  // recurse until the stack ran out. Report once, then die quietly.
  static thread_local bool reporting = false;
  if (!reporting) {
    reporting = true;
    char location[512];
    base::FormatTo(location, sizeof(location), "fatal check: {}:{}", file, line);
    assert_handler(location, file, "", message);
    reporting = false;
  }
  base::TerminateAbnormally();
}
}  // namespace detail

void SetCheckHandler(CheckHandler handler) {
  assert_handler = handler;
}
}  // namespace base
