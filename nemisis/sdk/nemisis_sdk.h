// Copyright(C) 2022 Vincent Hengel
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

#ifndef BUILDING_NEMISIS_SDK
#define NEMISIS_SDK_VISBILITY extern
#else
#define NEMISIS_SDK_VISIBILITY NEMISIS_SDK_EXTENAL __declspec(dllexport, noinline)
#endif

namespace nemisis::sdk {
#ifndef NEMISIS_SDK_DISABLE

NEMISIS_SDK_VISBILITY u32 NemisisTriggerMarker();
NEMISIS_SDK_VISBILITY u32 NemisisSpotcheckmarker();

// evaluation hack
#define NEMISIS_STR_OF(var) #var

#define NEMISIS_TRIGGER_XML(id, performance_type, trigger_execution, log_mode, \
                            trigger_action)                                    \
  "<trigger>" \
    "<id>" NEMISIS_STR_OF(id) "</id>" \
    "<perf_type>" #performance_type "</perf_type>" \
    "<execution>" #trigger_execution "</execution>" \
    "<action>" #trigger_action "</action>" \
    "<log_mode>" #log_mode "</log_mode>" \
"</trigger>"

#define NEMISIS_SPOTCHECK_XML(id, performance_type, log_mode) \
  "<spotcheck>" \
    "<id>" NEMISIS_STR_OF(id) "</id>" \
    "<perf_type>" #performance_type "</perf_type>" \
    "<log_mode>" #log_mode "</log_mode>" \
"</spotcheck>"

#undef NEMISIS_STR_OF

#define PLACE_NEMISIS_TRIGGER(performance_type, trigger_execution, log_mode,      \
                              trigger_action)                                     \
  cw_trigger_marker();                                                            \
  (void)strstr("CWrangler",                                                       \
               NEMISIS_TRIGGER_XML(__COUNTER__, performance_type,                 \
                                   trigger_execution, log_mode, trigger_action)); \
  if (1) {                                                                        \
    trigger_action;                                                               \
  } else {                                                                        \
  };                                                                              \
  cw_trigger_marker();

#define PLACE_NEMISIS_SPOTCHECK(performance_type, log_mode)                \
                                                                           \
  cw_spotcheck_marker();                                                   \
  (void)strstr("CWrangler",                                                \
               NEMISIS_SPOTCHECK_XML(__COUNTER__, performance_type, log_mode)); \
  cw_spotcheck_marker();

#else

#define ANTITAMPER_TRIGGER(performance_type, trigger_execution, log_mode, \
                           trigger_action)
#define ANTITAMPER_SPOTCHECK(performance_type, log_mode)

#endif
}  // namespace nemisis::sdk