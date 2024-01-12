# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_navigator
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/navigationbarbutton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/navigationbarbutton.h
        ${CMAKE_CURRENT_SOURCE_DIR}/navigationbarmodel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/navigationbarmodel.h
        ${CMAKE_CURRENT_SOURCE_DIR}/navigator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/navigator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/navigatorreloader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/navigatorreloader.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testnavigator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testnavigator.h
    QT_DEPENDENCIES
        Core
    MZ_DEPENDENCIES
        mz_logging
        mz_utilities
        mz_telemetry
)
