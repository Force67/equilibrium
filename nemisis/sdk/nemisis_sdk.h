// Copyright(C) 2022 Vincent Hengel
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

#ifndef BUILDING_NEMISIS_SDK
#define NEMISIS_SDK_VISBILITY extern
#else
#define NEMISIS_SDK_VISIBILITY extern __declspec(dllexport, noinline)
#endif

namespace nemisis::sdk {
#ifdef NEMISIS_SDK_DISABLE
#define NEMISIS_ANTITAMPER_TRIGGER(performance_type, trigger_execution, log_mode, \
                                   trigger_action)
#define NEMISIS_ANTITAMPER_SPOTCHECK(performance_type, log_mode)
#else

// dummy symbols, defined in nemisis_sdk.cc

// those must be placed at function start and end
NEMISIS_SDK_VISIBILITY u32 MarkTrigger();
NEMISIS_SDK_VISIBILITY u32 MarkSpotcheck();

// evaluation hack
#define NEMISIS_STR_OF(var) #var

#define NEMISIS_DEFINE_TRIGGER(id, performance_type, trigger_execution, log_mode, \
                               trigger_action)                                    \
  "@trigger\\id:" NEMISIS_STR_OF(id) "\\perf_type:" #performance_type             \
                                     "\\execution:" #trigger_execution            \
                                     "\\action:" #trigger_action                  \
                                     "\\log_mode" #log_mode

#define NEMISIS_DEFINE_SPOTCHECK(id, performance_type, log_mode)      \
  "@trigger\\id:" NEMISIS_STR_OF(id) "\\perf_type:" #performance_type \
                                     "\\log_mode" #log_mode

#undef NEMISIS_STR_OF

#define PLACE_NEMISIS_TRIGGER(performance_type, trigger_execution, log_mode,    \
                              trigger_action)                                   \
  nemisis::sdk::MarkTrigger();                                                  \
  (void)strstr("NEMISIS", NEMISIS_DEFINE_TRIGGER(__COUNTER__, performance_type, \
                                                 trigger_execution, log_mode,   \
                                                 trigger_action));              \
  if (1) {                                                                      \
    trigger_action;                                                             \
  } else {                                                                      \
  };                                                                            \
  nemisis::sdk::MarkTrigger()();

#define PLACE_NEMISIS_SPOTCHECK(performance_type, log_mode)                        \
                                                                                   \
  nemisis::sdk::MarkSpotcheck();                                                   \
  (void)strstr("NEMISIS",                                                          \
               NEMISIS_DEFINE_SPOTCHECK(__COUNTER__, performance_type, log_mode)); \
  nemisis::sdk::MarkSpotcheck();
#endif
}  // namespace nemisis::sdk