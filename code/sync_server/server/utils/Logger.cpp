// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Logger.h"

namespace noda::utils {

  static logcallback_t s_callback{ nullptr };

  static const char *GetLevelName(LogLevel log_level)
  {
#define LVL(x)      \
  case LogLevel::x: \
	return #x
	switch(log_level) {
	  LVL(Trace);
	  LVL(Info);
	  LVL(Warning);
	  LVL(Error);
	default:
	  return "???";
	}
#undef LVL
  }

  // pretty print me, please :-)
  void PrintLogMessageImpl(LogLevel ll, const char *text, const fmt::format_args &args)
  {
	const char *levelName = GetLevelName(ll);
	auto fmt = fmt::format("[{}]: {}", levelName, fmt::vformat(text, args));

	if(s_callback)
	  s_callback(fmt.c_str());
  }

  void SetLogCallback(logcallback_t callback)
  {
	s_callback = callback;
  }
} // namespace noda::utils