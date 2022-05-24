# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

option(BUILD_DUMMY "Build for the dummy platform" OFF)

project("Mozilla VPN" VERSION 2.9.0 LANGUAGES C CXX
        DESCRIPTION "A fast, secure and easy to use VPN. Built by the makers of Firefox."
        HOMEPAGE_URL "https://vpn.mozilla.org"
)

message("Configuring for ${CMAKE_GENERATOR}")
if(NOT (DEFINED CMAKE_CONFIGURATION_TYPES OR "${CMAKE_BUILD_TYPE}"))
    ## Ensure the build type is set for single-config generators.
    set(CMAKE_BUILD_TYPE "Debug" CACHE PATH "default build type" FORCE)
    message("Setting build type ${CMAKE_BUILD_TYPE}")
endif()

## Toolchain Setup
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.17)
    cmake_policy(SET CMP0099 OLD)
endif()

if(MSVC)
    include(src/cmake/msvc.cmake)
endif()

## Some workarounds for windows build quirks
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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 COMPONENTS Core)
message("Using Qt version ${QT_VERSION}")
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# VPN Client build targets
add_subdirectory(src)
add_subdirectory(glean)
add_subdirectory(lottie)
add_subdirectory(nebula)
add_subdirectory(translations)

# Testing build targets
option(BUILD_TESTING "Build the testing tree." OFF)
include(CTest)
add_custom_target(build_tests)
set_target_properties(build_tests PROPERTIES EXCLUDE_FROM_ALL $<NOT:$<BOOL:${BUILD_TESTING}>>)
add_subdirectory(tests/auth)
add_subdirectory(tests/nativemessaging)
add_subdirectory(tests/unit)
add_subdirectory(tests/qml)

# Web Extension build targets
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(extension)
endif()

# Extra platform stuff
if(WIN32)
    add_subdirectory(windows/installer)
    add_subdirectory(windows/split-tunnel)
    add_subdirectory(windows/tunnel)
endif()
