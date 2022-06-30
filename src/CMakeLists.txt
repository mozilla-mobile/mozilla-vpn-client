# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star/kremlin/minimal)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt_add_executable(mozillavpn MANUAL_FINALIZATION)

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

add_compile_definitions("$<$<CONFIG:Debug>:MVPN_DEBUG>")
add_compile_definitions("MVPN_$<UPPER_CASE:${MVPN_PLATFORM_NAME}>")
include(cmake/${MVPN_PLATFORM_NAME}.cmake)

add_subdirectory(crashreporter)
target_link_libraries(mozillavpn PRIVATE crashreporter)

qt_finalize_target(mozillavpn)
