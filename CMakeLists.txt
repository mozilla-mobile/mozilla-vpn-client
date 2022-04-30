cmake_minimum_required(VERSION 3.16)

project(mozillavpn VERSION 2.9.0 LANGUAGES C CXX)

## Ensure the build type is set
## TODO: Support multi-generators like XCode and Visual Studio
message("Configuring for ${CMAKE_GENERATOR}")
if(NOT "${CMAKE_BUILD_TYPE}")
    set(CMAKE_BUILD_TYPE "Debug" CACHE PATH "default build type" FORCE)
    message("Setting build type ${CMAKE_BUILD_TYPE}")
endif()

## Toolchain Setup
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    include(src/cmake/msvc.cmake)
endif()

if(WIN32 AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/unsigned"
        CACHE PATH "default install path" FORCE)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050F00)

# VPN Client build targets
add_subdirectory(src)
add_subdirectory(glean)
add_subdirectory(lottie)
add_subdirectory(nebula)
add_subdirectory(translations)

# Testing build targets
add_subdirectory(tests/auth)
add_subdirectory(tests/unit)
add_subdirectory(tests/qml)

# Web Extension build targets
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(extension)
endif()

# Extra platform stuff
if(WIN32)
    add_subdirectory(windows/installer)
    add_subdirectory(windows/tunnel)
endif()
