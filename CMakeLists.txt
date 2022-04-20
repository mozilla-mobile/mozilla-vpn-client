cmake_minimum_required(VERSION 3.16)

project(mozillavpn VERSION 2.9.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_subdirectory(tests/unit)
