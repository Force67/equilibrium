// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/process/process_metrics.h"

#include <unistd.h>

#include <base/filesystem/posix/small_file_posix.h>
#include <base/strings/char_algorithms.h>
#include <base/strings/format.h>
#include <base/strings/number_parse.h>
#include <base/strings/string_compare.h>

namespace base {
namespace {

// Matches "Key:<whitespace><number> kB" and returns the number in bytes.
// Returns false when the line is a different key.
bool ParseKilobyteLine(const char* line, const char* key, mem_size key_length,
                       mem_size& out) {
  if (Strncmp(line, key, key_length) != 0)
    return false;
  const char* cursor = line + key_length;
  i64 kib = 0;
  if (!ParseInteger(cursor, kib) || kib < 0)
    return false;
  out = static_cast<mem_size>(kib) * 1024u;
  return true;
}

}  // namespace

ProcessHandle GetCurrentProcessHandle() {
  return static_cast<ProcessHandle>(getpid());
}

bool QueryProcessMemoryUsage(ProcessHandle process, ProcessMemoryUsage& usage) {
  usage = {};
  if (process <= 0)
    return false;

  char path[64];
  const mem_size path_length =
      FormatTo(path, sizeof(path), "/proc/{}/status", process);
  if (path_length >= sizeof(path))
    return false;

  char status[8192];
  if (ReadSmallFile(path, status, sizeof(status)) < 0)
    return false;

  bool found_resident = false;
  for (const char* line = status; *line;) {
    mem_size bytes = 0;
    if (ParseKilobyteLine(line, "VmRSS:", 6, bytes)) {
      usage.resident_set_bytes = bytes;
      found_resident = true;
    } else if (ParseKilobyteLine(line, "VmHWM:", 6, bytes)) {
      usage.peak_resident_set_bytes = bytes;
    }
    const char* newline = FindChar(line, '\n');
    if (!newline)
      break;
    line = newline + 1;
  }

  if (found_resident)
    return true;
  usage = {};
  return false;
}

}  // namespace base
