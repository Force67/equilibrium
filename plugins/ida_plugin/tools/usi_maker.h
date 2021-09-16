// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

static constexpr uint64_t kInvalidUSI = UINT64_MAX;

// This is an implementation of the universal symbol standard, used by this program in order to
// provide a unique type identifier for given EA
uint64_t CreateUSI(const ea_t);

// create storage NODE
bool CreateReflectionSymbolTableEntry();
