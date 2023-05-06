# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

message("Configuring for ${CMAKE_GENERATOR}")
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT (IS_MULTI_CONFIG OR DEFINED CMAKE_BUILD_TYPE))
    ## Ensure the build type is set for single-config generators.
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "default build type" FORCE)
    message("Setting build type ${CMAKE_BUILD_TYPE}")
endif()

project("Mozilla VPN" VERSION 2.16.0 LANGUAGES C CXX
        DESCRIPTION "Mozilla VPN"
        HOMEPAGE_URL "https://vpn.mozilla.org"
)

## Some workarounds for platform build quirks
if(WIN32)
    ## CMake v3.20 has problems with race conditions in dependency generation.
    ## See: https://gitlab.kitware.com/cmake/cmake/-/issues/22014
    if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} VERSION_EQUAL 3.20)
        cmake_policy(SET CMP0116 OLD)
    endif()

    ## CMake also has trouble finding OpenSSL libraries on Windows, and may
    ## need some help.
    if(EXISTS "C:/MozillaVPNBuild/SSL" AND NOT DEFINED OPENSSL_ROOT_DIR)
        set(OPENSSL_ROOT_DIR "C:/MozillaVPNBuild/SSL")
        find_package(OpenSSL REQUIRED)
    endif()

    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/unsigned"
            CACHE PATH "default install path" FORCE)
    endif()
endif()
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
    if(IOS)
        enable_language(Swift)
    endif()

    ## Setup some defaults for code signing.
    if(NOT BUILD_OSX_APP_IDENTIFIER)
        set(BUILD_OSX_APP_IDENTIFIER org.mozilla.macos.FirefoxVPN CACHE STRING "OSX Application identifier")
    endif()
    if(NOT BUILD_IOS_APP_IDENTIFIER)
        set(BUILD_IOS_APP_IDENTIFIER org.mozilla.ios.FirefoxVPN CACHE STRING "iOS Application identifier")
    endif()
    if(NOT BUILD_IOS_GROUP_IDENTIFIER)
        set(BUILD_IOS_GROUP_IDENTIFIER group.org.mozilla.ios.Guardian CACHE STRING "iOS Group identifier")
    endif()
    if(NOT BUILD_VPN_DEVELOPMENT_TEAM)
        set(BUILD_VPN_DEVELOPMENT_TEAM 43AQ936H96 CACHE STRING "Mozilla VPN Development Team")
    endif()
    set(CMAKE_XCODE_GENERATE_SCHEME FALSE)
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${BUILD_VPN_DEVELOPMENT_TEAM})
    set(CMAKE_XCODE_ATTRIBUTE_GROUP_ID_IOS ${BUILD_IOS_GROUP_IDENTIFIER})

    if(IOS)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
    else()
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
    endif()
endif()

if(ANDROID)
    # Don't move, this provides OpenSSL on android.
    include(src/shared/cmake/android_openssl.cmake)
endif()

if(NOT DEFINED BUILD_ID)
    string(TIMESTAMP BUILD_ID "${PROJECT_VERSION_MAJOR}.%Y%m%d%H%M")
    message("Generated BUILD_ID: ${BUILD_ID}")
endif()

## Toolchain Setup
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.17)
    cmake_policy(SET CMP0099 OLD)
endif()

find_program(PYTHON_EXECUTABLE NAMES python3 python)
if(MSVC)
    include(src/apps/vpn/cmake/msvc.cmake)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    include(src/apps/vpn/cmake/clang.cmake)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER "Autogen")
set_property(GLOBAL PROPERTY AUTOMOC_TARGETS_FOLDER "Autogen")
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Autogen")

find_package(Qt6 COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickTest
    Test
    WebSockets
    Widgets
    Xml
    Svg
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" AND
   NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    find_package(Qt6 COMPONENTS
        NetworkAuth
        Sql
    )
endif()

message("Using Qt version ${Qt6_VERSION}")
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# VPN Client build targets
add_subdirectory(src)
# Glean.js
add_subdirectory(glean)
# Glean.rs
add_subdirectory(qtglean)
add_subdirectory(lottie)
add_subdirectory(nebula)
add_subdirectory(translations)

# Addons - Requires CMake 3.20 and later
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
    add_subdirectory(addons)
endif()

if(NOT CMAKE_CROSSCOMPILING)
    # Extra rustlang libraries
    add_subdirectory(extension)

    # Enable testing
    include(CTest)
    add_custom_target(build_tests)
    set_target_properties(build_tests PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        FOLDER "Tests"
    )

    # Unit Tests
    add_subdirectory(tests/nativemessaging EXCLUDE_FROM_ALL)
    add_subdirectory(tests/unit EXCLUDE_FROM_ALL)
    add_subdirectory(tests/qml EXCLUDE_FROM_ALL)
    add_subdirectory(src/apps/auth_tests EXCLUDE_FROM_ALL)
    add_subdirectory(src/apps/unit_tests EXCLUDE_FROM_ALL)

    # Dummy Testing Client
    add_subdirectory(tests/dummyvpn EXCLUDE_FROM_ALL)
endif()

# Extra platform targets
if(WIN32)
    add_subdirectory(windows/installer)
    add_subdirectory(windows/split-tunnel)
    add_subdirectory(windows/tunnel)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_subdirectory(macos/loginitem)
    add_subdirectory(macos/pkg)
elseif(IOS)
    add_subdirectory(ios/networkextension)
    if(BUILD_ADJUST_SDK_TOKEN)
        add_subdirectory(ios/adjust)
    endif()

    # A final post-processing hack to enable simulation builds.
    include(scripts/cmake/ios-arch-fixup.cmake)
endif()
