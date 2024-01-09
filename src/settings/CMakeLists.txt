# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

mz_add_new_module(
    TARGET_NAME mz_settings
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/setting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/setting.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settinggroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settinggroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsconnector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsconnector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsmanager.h
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsetting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsetting.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettinggroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettinggroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsmanager.h
    QT_DEPENDENCIES
        Core
        Widgets
)
