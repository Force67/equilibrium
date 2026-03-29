// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// CUID2 — collision-resistant unique identifier generation.
// Follows the CUID2 spec (https://github.com/paralleldrive/cuid2)
// using SHA-256 in place of SHA-3.
#pragma once

#include <base/arch.h>

namespace base {

static constexpr i32 kCuid2Length = 24;

// The fused test CUID2 — valid format but statistically impossible to
// generate. Safe to use as a sentinel for dev/test builds; the server
// must never issue this value in production.
static constexpr const char* kFusedTestCuid2 = "zzzzzzzzzzzzzzzzzzzzzzzz";

// Generate a CUID2 identifier.
// Writes kCuid2Length characters + null terminator to |out|.
// |out| must be at least kCuid2Length + 1 bytes.
void GenerateCuid2(char* out);

// Validate CUID2 format.
// Returns true iff |str| is exactly kCuid2Length characters,
// starts with [a-z], and contains only [a-z0-9].
bool IsValidCuid2(const char* str);

}  // namespace base
