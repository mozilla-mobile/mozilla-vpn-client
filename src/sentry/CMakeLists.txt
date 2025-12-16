# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Integrate Sentry with shared-sources
# This file should be included after shared-sources is defined
# The sentry library itself is built in 3rdparty/sentry.cmake

if(NOT BUILD_CRASHREPORTING)
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
    message("Integrating sentry for ${CMAKE_SYSTEM_NAME}")
    target_compile_definitions(shared-sources INTERFACE SENTRY_ENABLED)
    # Let's the app know we need to provide the upload client
    target_compile_definitions(shared-sources INTERFACE SENTRY_NONE_TRANSPORT)
    # Let sentry.h know we are using a static build
    target_compile_definitions(shared-sources INTERFACE SENTRY_BUILD_STATIC)

    # We need QML private headers for QML backtracing
    target_link_libraries(shared-sources INTERFACE Qt6::QmlPrivate)

    # Sentry support is given
    target_sources(shared-sources INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/sentryadapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sentryadapter.h
        ${CMAKE_SOURCE_DIR}/src/tasks/sentry/tasksentry.cpp
        ${CMAKE_SOURCE_DIR}/src/tasks/sentry/tasksentry.h
        ${CMAKE_SOURCE_DIR}/src/tasks/sentryconfig/tasksentryconfig.cpp
        ${CMAKE_SOURCE_DIR}/src/tasks/sentryconfig/tasksentryconfig.h
    )

    # Link to sentry target (built in 3rdparty/sentry.cmake)
    target_link_libraries(shared-sources INTERFACE sentry)

    # Platform-specific system libraries
    if(WIN32)
        target_link_libraries(shared-sources INTERFACE dbghelp.lib)
        target_link_libraries(shared-sources INTERFACE version.lib)
    endif()
endif()
