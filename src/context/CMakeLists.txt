# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Generate version header file
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

mz_add_new_module(
    TARGET_NAME mz_context
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/app.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/env.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/env.h
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/androidconstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/androidvpnactivity.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/androidvpnactivity.h
    IOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/iosconstants.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/iostunnelmanager.swift
        ${CMAKE_CURRENT_SOURCE_DIR}/iostunnelmessage.swift
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testenv.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testenv.h
    QT_DEPENDENCIES
        Core
        Network
        Widgets
        Quick
    MZ_DEPENDENCIES
        mz_settings
        mz_utilities
        mz_networking
        mz_taskscheduler
    ANDROID_DEPENDENCIES
        mz_navigator
)
