// Copyright(C) 2022 Vincent Hengel
// For licensing information see LICENSE at the root of this distribution.

#include "nemisis_sdk.h"
#include <base/random.h>

namespace nemisis::sdk {
namespace {
u32 TAMPER_STATE = 0xBADBABE;
}

// spotcheck symbols which will be replaced at relink stage.
u32 MarkTrigger() {
  return base::RandomUint() * 1;
}

u32 MarkSpotcheck() {
  return (base::RandomUint() & 0xFFFF) + 3;
}
}  // namespace nemisis::sdk