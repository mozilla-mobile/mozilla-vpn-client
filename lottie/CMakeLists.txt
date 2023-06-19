# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

project(lottie VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Lightweight scalable animations: QML Wrapper library"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_FOLDER "Lottie")
endif()
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.17)
    cmake_policy(SET CMP0099 OLD)
endif()

qt_add_library(lottie STATIC)

if(NOT MSVC AND NOT IOS)
  target_compile_options(lottie PRIVATE -Wall -Werror -Wno-conversion)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Qml Qml Quick QuickTest Test)
target_link_libraries(lottie PUBLIC lottieplugin Qt6::Core Qt6::Qml)
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
)

# This add's a new lib lottieplugin
qt_add_qml_module(lottie
    VERSION 0.1
    URI LottieAnimation 
    RESOURCE_PREFIX /lottie
    QML_FILES
        lib/lottie/LottieAnimation.qml
    RESOURCES
        lib/lottie/lottie.mjs
)

## This writes into the source directory to generate lottie.mjs. Which is frowned-upon. :`(
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie/lottie.mjs
    MAIN_DEPENDENCY lib/lottie/lottie_wrap.js.template
    DEPENDS lib/lottie/lottie.min.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/lib/lottie/lottie.mjs.cmake
)

## Include self-tests, unless we are being included in another project.
enable_testing()
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests/unit)
    add_subdirectory(tests/qml)
endif()
