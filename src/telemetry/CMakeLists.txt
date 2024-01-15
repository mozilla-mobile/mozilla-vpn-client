# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(EXTRA_DEPENDENCIES)
if (NOT ${MZ_PLATFORM_NAME} STREQUAL "wasm")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glean/qtglean)
    list(APPEND EXTRA_DEPENDENCIES qtglean)
endif()

if (${MZ_PLATFORM_NAME} STREQUAL "ios")
    include(${CMAKE_CURRENT_SOURCE_DIR}/glean/qtglean/ios.cmake)
endif()

mz_add_new_module(
    TARGET_NAME mz_telemetry
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/mzglean.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/mzglean.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/androidgleanbridge.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/androidgleanbridge.h
    IOS_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/generated/VPNMetrics.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/iosgleanbridge.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/iosgleanbridge.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/glean/iosgleanbridge.h
    QT_DEPENDENCIES
        Core
    MZ_DEPENDENCIES
        mz_logging
        mz_settings
        mz_utilities
        mz_feature
        mz_context
    IOS_DEPENDENCIES
        iosglean
    EXTRA_DEPENDENCIES
        ${EXTRA_DEPENDENCIES}
)
