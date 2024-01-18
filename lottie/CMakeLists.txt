# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.18)

project(lottie VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Lightweight scalable animations: QML Wrapper library"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_FOLDER "Lottie")
endif()

add_library(lottie STATIC)

mz_target_handle_warnings(lottie)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Qml Quick QuickTest Test)
target_link_libraries(lottie PUBLIC Qt6::Core Qt6::Qml)
target_include_directories(lottie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_sources(lottie PRIVATE
    lib/lottie.cpp
    lib/lottie.h
    lib/lottieprivate.cpp
    lib/lottieprivate.h
    lib/lottieprivatedocument.cpp
    lib/lottieprivatedocument.h
    lib/lottieprivatenavigator.cpp
    lib/lottieprivatenavigator.h
    lib/lottieprivatewindow.cpp
    lib/lottieprivatewindow.h
    lib/lottiestatus.h
    lib/lottie.qrc
)

## This writes into the source directory to generate lottie.mjs. Which is frowned-upon. :`(
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie/lottie.mjs
    MAIN_DEPENDENCY lib/lottie/lottie_wrap.js.template
    DEPENDS lib/lottie/lottie.min.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie/lottie.mjs.cmake
)

# We have no Test infra to run test's on android 
# so no need to spend time to run them VPN-4780
if(ANDROID)
    return()
endif()

## Include self-tests, unless we are being included in another project.
enable_testing()
add_subdirectory(tests/unit EXCLUDE_FROM_ALL)
if(NOT MSVC)
    add_subdirectory(tests/qml EXCLUDE_FROM_ALL)
endif()
