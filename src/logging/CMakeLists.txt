# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_logging
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/loghandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/loghandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/loglevel.h
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
