
// Copyright (C) Force67 2019

#include <base/time/time.h>
#include "nemisis_sdk.h"

namespace nemisis::sdk {
namespace {
u32 TAMPER_STATE = 0xBADBABE;
}

// spotcheck dummy function implementations
u32 NemisisTriggerMarker() {
  return (base::GetUnixTimeStamp() & 0xFFFFFFFF) + 1;
}

u32 NemisisSpotcheckmarker() {
  return (base::GetUnixTimeStamp() & 0xFFFFFFFF) + 1;
}
}  // namespace nemisis::sdk