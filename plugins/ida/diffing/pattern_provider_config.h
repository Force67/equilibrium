// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO: think about moving this to diffing component as well.
namespace pattern_config {

// how many instructions the generator can try to use to build a pattern
constexpr size_t kInstructionLimit = 100u;
// max length of a signature, including spaces and question marks
constexpr size_t kSignatureMaxLength = 75u;
// min length of a signature in bytes
constexpr size_t kMinSignatureByteLength = 5u;
// max displacement size in bytes
constexpr size_t kDisplacementStepSize = 100u;
// max reference count to be considered this is done to limit at
// runtime where it scales up pretty badly if we have lots of references :(
constexpr size_t kMaxRefCountAnalysisDepth = 10u;

// Does this even make sense?
// Bounds checking?!?
static_assert(kDisplacementStepSize <= INT8_MAX,
              "Displacement bounds exceeded");
}