// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#if defined(BUILDING_DLL)
#if defined(OS_WIN)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __declspec(dllexport)
#else
#define BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(BASE_IMPLEMENTATION)
#define BASE_EXPORT __attribute__((visibility("default")))
#else
#define BASE_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif
#else
// Define it as nothing
#define BASE_EXPORT
#endif