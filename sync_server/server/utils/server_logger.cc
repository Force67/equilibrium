// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "server_logger.h"

namespace sync_server::utils {

static logcallback_t s_callback{nullptr};

// pretty print me, please :-)
void PrintLogMessageImpl(LogLevel ll,
                         const char* text,
                         const fmt::format_args& args) {
  auto fmt = fmt::vformat(text, args);

  if (s_callback)
    s_callback(ll, fmt.c_str());
}

void SetLogCallback(logcallback_t callback) {
  s_callback = callback;
}
}  // namespace sync_server::utils