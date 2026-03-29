// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <cstdio>
#include <cstring>

namespace base {

namespace {
const char* const kLevelToNames[] = {"trace",   "debug", "info",
                                     "warning", "error", "fatal"};
static_assert(sizeof(kLevelToNames) / sizeof(const char*) ==
                  static_cast<size_t>(LogLevel::kAll),
              "Mapping mismatch");

void DefaultLogHandler(void* /*user_pointer*/,
                       const char* channel_name,
                       LogLevel ll,
                       const char* msg) {
  std::fprintf(stderr, "[%s] %s: %s\n", channel_name ? channel_name : "?",
               LogLevelToName(ll), msg ? msg : "");
}

constinit struct {
  void* user_pointer;
  LogHandler callback;
} log_data{nullptr, DefaultLogHandler};

// ── Per-channel filtering ──────────────────────────────────────────
constexpr int kMaxChannels = 64;
struct ChannelEntry {
  const char* name;
  LogLevel min_level;
};
ChannelEntry channel_table[kMaxChannels] = {};
int channel_count = 0;
LogLevel global_min_level = LogLevel::kTrace;

}  // namespace

const char* LogLevelToName(LogLevel level) noexcept {
  return kLevelToNames[static_cast<size_t>(level)];
}

void SetLogHandler(LogHandler callback, void* user_pointer) noexcept {
  if (!callback)
    callback = DefaultLogHandler;
  log_data = {user_pointer, callback};
}

void SetLogInstance(void* user_pointer) {
  log_data.user_pointer = user_pointer;
}

void SetChannelMinLevel(const char* channel_name,
                        LogLevel min_level) noexcept {
  // Update existing entry if present.
  for (int i = 0; i < channel_count; ++i) {
    if (std::strcmp(channel_table[i].name, channel_name) == 0) {
      channel_table[i].min_level = min_level;
      return;
    }
  }
  // Add new entry.
  if (channel_count < kMaxChannels) {
    channel_table[channel_count++] = {channel_name, min_level};
  }
}

void SetGlobalMinLevel(LogLevel min_level) noexcept {
  global_min_level = min_level;
}

bool ShouldLog(const char* channel_name, LogLevel level) noexcept {
  if (static_cast<int>(level) < static_cast<int>(global_min_level))
    return false;
  if (channel_name) {
    for (int i = 0; i < channel_count; ++i) {
      if (std::strcmp(channel_table[i].name, channel_name) == 0) {
        return static_cast<int>(level) >=
               static_cast<int>(channel_table[i].min_level);
      }
    }
  }
  return true;
}

namespace detail {
void WriteLogMessage(const char* channel_name, LogLevel ll, const char* text) {
  if (!ShouldLog(channel_name, ll))
    return;
  log_data.callback(log_data.user_pointer, channel_name, ll, text);
}
}  // namespace detail
}  // namespace base
