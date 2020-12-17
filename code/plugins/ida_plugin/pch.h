// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO: MSVC version check
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

#include <auto.hpp>
#include <bytes.hpp>
#include <enum.hpp>
#include <ida.hpp>
#include <idp.hpp>
#include <kernwin.hpp>
#include <loader.hpp>
#include <name.hpp>
#include <netnode.hpp>
#include <struct.hpp>

#pragma warning(pop)

#ifdef wait
#undef wait
#endif