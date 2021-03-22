// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <flatbuffers/flatbuffers.h>

namespace sync {
using FbsBuffer = flatbuffers::FlatBufferBuilder;

// Flat buffer aliases
template <typename T>
using FbsRef = flatbuffers::Offset<T>;
using FbsStringRef = flatbuffers::Offset<flatbuffers::String>;
}