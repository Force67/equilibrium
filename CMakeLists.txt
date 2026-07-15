# Copyright (C) 2026 Vincent Hengel.
# For licensing information see LICENSE at the root of this distribution.
#
# CMake build for the equilibrium base library, alongside the premake5
# build. Embedders add_subdirectory this and link equilibrium::base; the
# unit tests only build when EQ_BUILD_TESTS is on (default when equilibrium
# is the top-level project).

cmake_minimum_required(VERSION 3.16)
project(Equilibrium CXX C)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(eq_is_top_level ON)
else()
  set(eq_is_top_level OFF)
endif()

option(EQ_BUILD_TESTS "Build the base unit tests" ${eq_is_top_level})
option(EQ_USE_STD_ATOMIC "Build base::Atomic on top of std::atomic" OFF)
# file_util_win.cc formats with fmtlib, which equilibrium does not vendor.
# Windows embedders point this at an fmt checkout (e.g. zetanet's
# vendor/fmtlib); other platforms don't need it.
set(EQ_FMTLIB_DIR "" CACHE PATH "Path to an fmtlib checkout (Windows only)")

# The base headers use C++23 (deducing this, constexpr extensions).
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 23)
  set(CMAKE_CXX_STANDARD 23)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EQ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# --- base library ---
file(GLOB_RECURSE EQ_BASE_SOURCES "${EQ_ROOT}/base/*.cc")
list(FILTER EQ_BASE_SOURCES EXCLUDE REGEX "_test\\.cc$")
list(FILTER EQ_BASE_SOURCES EXCLUDE REGEX "_bench\\.cc$")
list(FILTER EQ_BASE_SOURCES EXCLUDE REGEX "memory_unittests_main\\.cc$")

# Platform suffix and directory filters, mirroring build/platform_files.lua:
# windows = {win, win32}, linux = {linux, posix}, macosx = {mac, posix}.
if(WIN32)
  set(eq_excluded_platforms linux posix mac)
elseif(APPLE)
  set(eq_excluded_platforms win win32 linux)
else()
  set(eq_excluded_platforms win win32 mac)
endif()
foreach(platform ${eq_excluded_platforms})
  list(FILTER EQ_BASE_SOURCES EXCLUDE REGEX "_${platform}\\.cc$")
  list(FILTER EQ_BASE_SOURCES EXCLUDE REGEX "/${platform}/")
endforeach()

add_library(eq_base STATIC ${EQ_BASE_SOURCES})
add_library(equilibrium::base ALIAS eq_base)
target_include_directories(eq_base PUBLIC "${EQ_ROOT}" "${EQ_ROOT}/base")
target_compile_definitions(eq_base PRIVATE
  BASE_IMPLEMENTATION
  TRACY_HAS_CALLSTACK
  "PROJECT_NAME=\"base\"")
# The premake Debug configuration defines this; checks only trap when it is
# set (CHECK_BREAK in compiler.h).
target_compile_definitions(eq_base PUBLIC "$<$<CONFIG:Debug>:CONFIG_DEBUG>")
set_target_properties(eq_base PROPERTIES
  CXX_STANDARD 23
  POSITION_INDEPENDENT_CODE ON)

if(EQ_USE_STD_ATOMIC)
  target_compile_definitions(eq_base PUBLIC BASE_USE_STD_ATOMIC=1)
endif()

if(WIN32)
  # UNICODE: select the wide (W) Win32 entry points so they accept the wchar_t
  # Path buffers, and make <windows.h>'s LOGFONT/MENUITEMINFO/WIN32_FIND_DATA
  # resolve to the same *W types minwin.h forward-declares (otherwise the two
  # disagree and MSVC reports "redefinition; different basic types").
  target_compile_definitions(eq_base PUBLIC OS_WIN NOMINMAX UNICODE _UNICODE)
  target_link_libraries(eq_base PRIVATE psapi)
  if(MSVC)
    target_compile_options(eq_base PUBLIC "/utf-8")
  endif()
  if(NOT EQ_FMTLIB_DIR)
    message(FATAL_ERROR "Windows base needs fmtlib, set EQ_FMTLIB_DIR")
  endif()
elseif(APPLE)
  target_compile_definitions(eq_base PUBLIC OS_MAC OS_POSIX)
else()
  target_compile_definitions(eq_base PUBLIC OS_LINUX OS_POSIX)
  find_package(Threads REQUIRED)
  target_link_libraries(eq_base PUBLIC Threads::Threads)
endif()

if(EQ_FMTLIB_DIR)
  add_library(eq_fmtlib STATIC
    "${EQ_FMTLIB_DIR}/src/format.cc"
    "${EQ_FMTLIB_DIR}/src/os.cc")
  target_include_directories(eq_fmtlib PUBLIC "${EQ_FMTLIB_DIR}/include")
  set_target_properties(eq_fmtlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(MSVC)
    # fmt 11+ static_asserts that its own sources compile with /utf-8.
    target_compile_options(eq_fmtlib PRIVATE "/utf-8")
  endif()
  target_link_libraries(eq_base PUBLIC eq_fmtlib)
endif()

# Per-arch baselines, matching what the premake build enables on x86. The
# x86 set carries the inline assembly requirements (spinlock pause, TSX);
# aarch64 turns on the CRC extension so CRC32C uses the hardware path.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64" AND NOT MSVC)
  set(eq_arch_options -mavx2 -mwaitpkg -mrtm)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64" AND NOT MSVC)
  set(eq_arch_options -march=armv8-a+crc)
else()
  set(eq_arch_options "")
endif()
target_compile_options(eq_base PRIVATE ${eq_arch_options})

# --- unit tests ---
if(EQ_BUILD_TESTS)
  enable_testing()

  set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  add_subdirectory("${EQ_ROOT}/external/googletest" EXCLUDE_FROM_ALL)

  file(GLOB_RECURSE EQ_BASE_TEST_SOURCES "${EQ_ROOT}/base/*_test.cc")
  list(FILTER EQ_BASE_TEST_SOURCES EXCLUDE REGEX "/allocator/")
  foreach(platform ${eq_excluded_platforms})
    list(FILTER EQ_BASE_TEST_SOURCES EXCLUDE REGEX "_${platform}_test\\.cc$")
    list(FILTER EQ_BASE_TEST_SOURCES EXCLUDE REGEX "/${platform}/")
  endforeach()

  add_executable(base_unittests
    ${EQ_BASE_TEST_SOURCES}
    "${EQ_ROOT}/build/generic_test_main.cc")
  target_compile_definitions(base_unittests PRIVATE "PROJECT_NAME=\"base\"")
  target_compile_options(base_unittests PRIVATE ${eq_arch_options})
  target_link_libraries(base_unittests PRIVATE eq_base gtest gmock)
  add_test(NAME base_unittests COMMAND base_unittests)

  # The allocator suite compiles the memory subsystem standalone with extra
  # debug verification, driven by its own harness instead of gtest.
  file(GLOB_RECURSE EQ_MEM_TEST_SOURCES "${EQ_ROOT}/base/allocator/*.cc")
  list(FILTER EQ_MEM_TEST_SOURCES EXCLUDE REGEX "_test\\.cc$")
  list(FILTER EQ_MEM_TEST_SOURCES EXCLUDE REGEX "_bench\\.cc$")
  foreach(platform ${eq_excluded_platforms})
    list(FILTER EQ_MEM_TEST_SOURCES EXCLUDE REGEX "_${platform}\\.cc$")
    list(FILTER EQ_MEM_TEST_SOURCES EXCLUDE REGEX "/${platform}/")
  endforeach()

  add_executable(base_memory_unittests
    ${EQ_MEM_TEST_SOURCES}
    "${EQ_ROOT}/base/containers/linked_list.cc"
    "${EQ_ROOT}/base/threading/spinning_mutex.cc")
  target_include_directories(base_memory_unittests PRIVATE
    "${EQ_ROOT}" "${EQ_ROOT}/base")
  target_compile_definitions(base_memory_unittests PRIVATE
    BASE_IMPLEMENTATION
    BASE_STRIP_BUGCHECK
    BASE_MEM_CORE_DEBUG
    X2E_MEMORY_TRACKING
    "PROJECT_NAME=\"base\"")
  if(NOT WIN32 AND NOT APPLE)
    target_compile_definitions(base_memory_unittests PRIVATE OS_LINUX OS_POSIX)
    target_link_libraries(base_memory_unittests PRIVATE Threads::Threads)
  elseif(WIN32)
    target_compile_definitions(base_memory_unittests PRIVATE OS_WIN NOMINMAX)
  endif()
  add_test(NAME base_memory_unittests COMMAND base_memory_unittests)
endif()
