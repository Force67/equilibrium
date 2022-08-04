// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/version.h>
#include <base/memory/memory_literals.h>

namespace program_database {
using namespace base::memory_literals;

constexpr u32 kDbPageSize = 64_kib;
constexpr u16 kSectionAlignment = 16;
// v 1.0.0
static u16 kCurrentTkDbVersion{base::MakeVersionKey<u16>(10, 1, 0, 0)};
}  // namespace program_database
