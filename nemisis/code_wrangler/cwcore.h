#pragma once

// Copyright (C) Force67 2019

#ifdef _WIN32
#ifdef COMPILING_CORE
#define CWAPI __declspec(dllexport)
#define CWAPI_C extern "C" __declspec(dllexport)
#else
#define CWAPI __declspec(dllimport)
#define CWAPI_C extern "C" __declspec(dllimport)
#endif
#endif

#include <cstdint>
#include <memory>
#include <utl/File.h>

#include <fmtdef.h>

// automatically determine image type
CWAPI_C cwImageType cwChooseImageType(const char *inputFile);

// regular link operation
CWAPI_C bool cwLinkImage(cwImageType, const char *outputDir, const char *commands);

// code wrangler extended relink operation
CWAPI_C bool cwRelinkImage(cwImageType, const char* inputFile, const char *outputDir, const char *outputFileName);