
// Copyright (C) Force67 2019

#include "cwsdk.h"

uint32_t TAMPER_STATE = 0xBADBABE;

// spotcheck dummy function implementations
__time64_t cw_trigger_marker() { return _time64(0) + 1; } \
__time64_t cw_spotcheck_marker() { return _time64(0) + 2; }