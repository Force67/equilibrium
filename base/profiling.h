// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/logging.h>

#if defined(ENABLE_PROFILE)

#ifndef TRACY_ENABLE
#define TRACY_ENABLE // just to make sure
#endif

#include <external/tracy/Tracy.hpp>

#define BASE_PROFILE ZoneScoped
#define BASE_PROFILE_FRAME(x) FrameMark
#define BASE_PROFILE_SECTION(x) ZoneScopedN(x)
#define BASE_PROFILE_TAG(y, x) ZoneText(x, strlen(x))
#define BASE_PROFILE_LOG(text, size) TracyMessage(text, size)
#define BASE_PROFILE_VALUE(text, value) TracyPlot(text, value)
#define BASE_PROFILE_ALLOCATION(size) TracyCAllocS(p, size, 12)

#else

#define BASE_PROFILE
#define BASE_PROFILE_FRAME(x)
#define BASE_PROFILE_SECTION(x)
#define BASE_PROFILE_TAG(y, x)
#define BASE_PROFILE_LOG(text, size)
#define BASE_PROFILE_VALUE(text, value)

#endif