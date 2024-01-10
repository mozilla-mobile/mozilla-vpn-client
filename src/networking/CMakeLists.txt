# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_networking
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/networkmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/networkmanager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/networkrequest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/networkrequest.h
        ${CMAKE_CURRENT_SOURCE_DIR}/simplenetworkmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/simplenetworkmanager.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testnetworkmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testnetworkmanager.h
    QT_DEPENDENCIES
        Core
        Network
    MZ_DEPENDENCIES
        mz_settings
        mz_context
        mz_utilities
)
