# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(MZ_SETTINGS_INCLUDE_DIRECTORIES "")
set(MZ_SETTINGS_EXTRA_DEPENDENCIES "")

if(${MZ_PLATFORM_NAME} STREQUAL "linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(libsecret REQUIRED IMPORTED_TARGET libsecret-1)

    list(APPEND MZ_SETTINGS_INCLUDE_DIRECTORIES ${libsecret_INCLUDE_DIRS})
    list(APPEND MZ_SETTINGS_EXTRA_DEPENDENCIES PkgConfig::libsecret)
endif()

mz_add_new_module(
    TARGET_NAME mz_settings
    INCLUDE_DIRECTORIES
        ${MZ_SETTINGS_INCLUDE_DIRECTORIES}
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/setting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/setting.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settinggroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settinggroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsconnector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsconnector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsmanager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsholder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settingsholder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/cryptosettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/cryptosettings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/settingslist.h
    ANDROID_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/androidcryptosettings.cpp
    IOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/macoscryptosettings.mm
    MACOS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/macoscryptosettings.mm
    WINDOWS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/windowscryptosettings.cpp
    LINUX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/linuxcryptosettings.cpp
    WASM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/dummycryptosettings.cpp
    DUMMY_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings/dummycryptosettings.cpp
    TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsetting.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsetting.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettinggroup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettinggroup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsmanager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsmanager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsholder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/testsettingsholder.h
    QT_DEPENDENCIES
        Core
        Widgets
    MZ_DEPENDENCIES
        mz_logging
        mz_utilities
        mz_crypto
        mz_feature
    EXTRA_DEPENDENCIES
        ${MZ_SETTINGS_EXTRA_DEPENDENCIES}
)
