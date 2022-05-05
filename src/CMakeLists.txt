# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(NOT DEFINED BUILD_ID)
    string(TIMESTAMP BUILD_ID "${mozillavpn_VERSION_MAJOR}.%Y%m%d%H%M")
    message("Generated BUILD_ID: ${BUILD_ID}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Test)
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)
find_package(Qt6 REQUIRED COMPONENTS NetworkAuth)
find_package(Qt6 REQUIRED COMPONENTS WebSockets)

add_executable(mozillavpn)
add_compile_definitions("$<$<CONFIG:Debug>:MVPN_DEBUG>")

target_link_libraries(mozillavpn PRIVATE
    Qt6::NetworkAuth
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

target_link_libraries(mozillavpn PRIVATE glean lottie nebula translations)

include(cmake/sources.cmake)

if(${BUILD_DUMMY})
    set(MVPN_PLATFORM_NAME "dummy")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(MVPN_PLATFORM_NAME "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(MVPN_PLATFORM_NAME "windows")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MVPN_PLATFORM_NAME "macos")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    set(MVPN_PLATFORM_NAME "android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(MVPN_PLATFORM_NAME "ios")
endif()

add_compile_definitions("MVPN_$<UPPER_CASE:${MVPN_PLATFORM_NAME}>")
include(cmake/${MVPN_PLATFORM_NAME}.cmake)

add_subdirectory(crashreporter)
target_link_libraries(mozillavpn PRIVATE crashreporter)

set_target_properties(mozillavpn PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
