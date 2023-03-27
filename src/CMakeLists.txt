# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(${MAIN_TARGET} MANUAL_FINALIZATION)

if(NOT MSVC AND NOT IOS)
  target_compile_options(${MAIN_TARGET} PRIVATE -Wall -Werror -Wno-conversion)
endif()

target_link_libraries(${MAIN_TARGET} PRIVATE
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Svg
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(${MAIN_TARGET} PRIVATE
        Qt6::NetworkAuth
    )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(MZ_PLATFORM_NAME "linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(MZ_PLATFORM_NAME "windows")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(MZ_PLATFORM_NAME "macos")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    set(MZ_PLATFORM_NAME "android")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(MZ_PLATFORM_NAME "ios")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(MZ_PLATFORM_NAME "wasm")
endif()

include(shared/sources.cmake)
include(apps/vpn/cmake/sources.cmake)

target_link_libraries(${MAIN_TARGET} PRIVATE
    shared-sources
    ${MAIN_TARGET}-sources
    glean
    lottie
    nebula
    translations_vpn
    qtglean
)

target_link_libraries(${MAIN_TARGET} PUBLIC ${CMAKE_DL_LIBS})

target_compile_definitions(${MAIN_TARGET} PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_compile_definitions(${MAIN_TARGET} PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")
include(apps/vpn/cmake/${MZ_PLATFORM_NAME}.cmake)


if(NOT CMAKE_CROSSCOMPILING)
    target_compile_definitions(${MAIN_TARGET} PUBLIC MVPN_WEBEXTENSION)
endif()

qt6_add_qml_module(${MAIN_TARGET}
  URI Mozilla.Shared.qmlcomponents
  VERSION 1.0
)

qt_finalize_target(${MAIN_TARGET})
