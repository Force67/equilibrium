#pragma once

// Copyright (C) Force67 2019

#include <cstdint>
#include <string.h>
#include <time.h>  

#define CWAPI

#ifdef __cplusplus
#define CW_EXTERNAL extern "C"
#else
#define CW_EXTERNAL
#endif

#ifndef CWAPI
#define CWVISBILITY CW_EXTERNAL extern
#else
#define CWVISBILITY CW_EXTERNAL __declspec (dllexport, noinline)
#endif

extern uint32_t TAMPER_STATE;

#ifndef CW_DISABLE

CWVISBILITY __time64_t cw_trigger_marker();
CWVISBILITY __time64_t cw_spotcheck_marker();

// evaluation hack
#define CW_STR_OF(var) #var

#define CW_TRIGGER_XML(id, performance_type, trigger_execution, log_mode, trigger_action) \
"<trigger>" \
    "<id>" CW_STR_OF(id) "</id>" \
    "<perf_type>" #performance_type "</perf_type>" \
    "<execution>" #trigger_execution "</execution>" \
    "<action>" #trigger_action "</action>" \
    "<log_mode>" #log_mode "</log_mode>" \
"</trigger>"

#define CW_SPOTCHECK_XML(id, performance_type, log_mode) \
"<spotcheck>" \
    "<id>" CW_STR_OF(id) "</id>" \
    "<perf_type>" #performance_type "</perf_type>" \
    "<log_mode>" #log_mode "</log_mode>" \
"</spotcheck>"

#define CW_TRIGGER(performance_type, trigger_execution, log_mode, trigger_action) \
cw_trigger_marker(); \
(void)strstr("CWrangler", CW_TRIGGER_XML(__COUNTER__, performance_type, trigger_execution, log_mode, trigger_action)); \
if (1) { trigger_action; } else { }; \
cw_trigger_marker(); \

#define CW_SPOTCHECK(performance_type, log_mode) \
    \
cw_spotcheck_marker(); \
(void)strstr("CWrangler", CW_SPOTCHECK_XML(__COUNTER__, performance_type, log_mode)); \
cw_spotcheck_marker(); \

#else

#define ANTITAMPER_TRIGGER(performance_type, trigger_execution, log_mode, trigger_action)
#define ANTITAMPER_SPOTCHECK(performance_type, log_mode)

#endif