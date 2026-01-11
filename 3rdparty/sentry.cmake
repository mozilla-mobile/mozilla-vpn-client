# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Build Sentry library

if(NOT BUILD_CRASHREPORTING)
    message("Sentry disabled by commandline")
    return()
endif()

# Defines which OS builds can include sentry
set(SENTRY_SUPPORTED_OS "Windows" "Darwin" "Android" "iOS" "Linux")

LIST(FIND SENTRY_SUPPORTED_OS ${CMAKE_SYSTEM_NAME} _SUPPORTED)

## Remove support for android 32bit.
## It's currently broken. see: VPN-3332
if(CMAKE_ANDROID_ARCH STREQUAL "x86")
    set(_SUPPORTED -1)
elseif(CMAKE_ANDROID_ARCH STREQUAL "arm")
    set(_SUPPORTED -1)
endif()

if(${_SUPPORTED} GREATER -1)
    message("Building sentry for ${CMAKE_SYSTEM_NAME}")

    # Configure platform-specific backend
    if(APPLE)
        include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
        set(SENTRY_BACKEND breakpad)
    elseif(WIN32)
        set(SENTRY_BACKEND breakpad)
    elseif(ANDROID)
        set(SENTRY_BACKEND inproc)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(SENTRY_BACKEND breakpad)
    endif()

    FetchContent_Declare(sentry SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/sentry")

    # Set sentry-specific options before calling FetchContent_MakeAvailable
    set(SENTRY_TRANSPORT none CACHE STRING "Disable sentry transport" FORCE)
    set(SENTRY_BUILD_TESTS OFF CACHE BOOL "Disable sentry tests" FORCE)
    set(SENTRY_BUILD_EXAMPLES OFF CACHE BOOL "Disable sentry examples" FORCE)
    set(SENTRY_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static sentry" FORCE)
    set(SENTRY_BACKEND ${SENTRY_BACKEND} CACHE STRING "Sentry backend" FORCE)

    FetchContent_MakeAvailable(sentry)
else()
    # Sentry is not supported on this platform
    message("Sentry supported OS -> ${SENTRY_SUPPORTED_OS}")
    message("Cannot build sentry for ${CMAKE_SYSTEM_NAME}")
endif()
