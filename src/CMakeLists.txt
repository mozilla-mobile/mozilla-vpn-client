# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(mozillavpn MANUAL_FINALIZATION)

mz_target_handle_warnings(mozillavpn)

target_link_libraries(mozillavpn PRIVATE
    Qt6::Quick
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Svg
)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(mozillavpn PRIVATE
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

include(cmake/shared-sources.cmake)
include(cmake/sources.cmake)

target_link_libraries(mozillavpn PRIVATE
    shared-sources
    mozillavpn-sources
    lottie
    nebula
    translations
    qtglean
)

target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

if ((${CMAKE_SYSTEM_NAME} STREQUAL "Android") OR (${CMAKE_SYSTEM_NAME} STREQUAL "iOS"))
    target_compile_definitions(mozillavpn PRIVATE "MZ_MOBILE")
endif()

target_compile_definitions(mozillavpn PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_compile_definitions(mozillavpn PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")
include(cmake/${MZ_PLATFORM_NAME}.cmake)


if(NOT CMAKE_CROSSCOMPILING)
    target_compile_definitions(mozillavpn PUBLIC MVPN_WEBEXTENSION)
endif()

add_subdirectory(ui)
target_link_libraries(mozillavpn PRIVATE mozillavpn-ui mozillavpn-uiplugin)

qt_finalize_target(mozillavpn)
