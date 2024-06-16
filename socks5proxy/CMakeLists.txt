# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

project(socks5proxy VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "A simple Socks5Proxy implementation"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_FOLDER "Socks5Proxy")
endif()
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.17)
    cmake_policy(SET CMP0099 OLD)
endif()

add_library(socks5proxy STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
target_link_libraries(socks5proxy PUBLIC Qt6::Core Qt6::Network)
target_include_directories(socks5proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_sources(socks5proxy PRIVATE
    src/socks5.cpp
    src/socks5.h
    src/socks5client.cpp
    src/socks5client.h
)
