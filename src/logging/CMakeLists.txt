# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(INCLUDE_DIRECTORIES)
if(${MZ_PLATFORM_NAME} STREQUAL "ios")
    find_package(Qt6::Gui)
    set(INCLUDE_DIRECTORIES ${Qt6Gui_PRIVATE_INCLUDE_DIRS})
endif()

mz_add_new_module(
    TARGET_NAME mz_logging
    INCLUDE_DIRECTORIES
        ${INCLUDE_DIRECTORIES}
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/loghandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/loghandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/loglevel.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/androidlogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/androidlogger.cpp
    IOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/ioslogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ioslogger.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/ioslogger.swift
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testlogger.cpp
    QT_DEPENDENCIES
        Core
        Widgets
    MZ_DEPENDENCIES
        mz_utilities
        mz_feature
)
