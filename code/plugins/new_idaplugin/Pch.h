// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// TODO: MSVC version check
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

#include <ida.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <idp.hpp>
#include <auto.hpp>
#include <kernwin.hpp>
#include <loader.hpp>
#include <netnode.hpp>
#include <name.hpp>
#include <enum.hpp>
#include <struct.hpp>

#pragma warning(pop)

#ifdef wait
#undef wait
#endif