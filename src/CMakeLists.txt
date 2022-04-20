# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_definitions(-DAPP_VERSION=\"${CMAKE_PROJECT_VERSION}\")
if(DEFINED BUILD_ID)
    add_definitions(-DBUILD_ID=\"${BUILD_ID}\")
else()
    add_definitions(-DBUILD_ID=\"1234\") ## TODO: FIX ME!
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Test)
find_package(Qt6 REQUIRED COMPONENTS Qml Quick)
find_package(Qt6 REQUIRED COMPONENTS NetworkAuth)
find_package(Qt6 REQUIRED COMPONENTS WebSockets)

add_executable(mozillavpn)

target_link_libraries(mozillavpn PRIVATE
    Qt6::NetworkAuth
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
)

target_link_libraries(mozillavpn PRIVATE glean lottie nebula translations)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DMVPN_DEBUG)
endif()

include(cmake/sources.cmake)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Dummy")
    add_definitions(-DMVPN_DUMMY)
    include(cmake/dummy.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DMVPN_LINUX)
    include(cmake/linux.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_definitions(-DMVPN_WINDOWS)
    include(cmake/windows.cmake)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_definitions(-DMVPN_MACOS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_definitions(-DMVPN_ANDROID)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    add_definitions(-DMVPN_IOS)
endif()

add_subdirectory(crashreporter)
target_link_libraries(mozillavpn PRIVATE crashreporter)

set_target_properties(mozillavpn PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
