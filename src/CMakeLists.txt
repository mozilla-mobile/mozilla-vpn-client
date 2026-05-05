# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(mozillavpn MANUAL_FINALIZATION)

mz_target_handle_warnings(mozillavpn)

find_package(Qt6 REQUIRED COMPONENTS Svg)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate QmlPrivate)
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
add_subdirectory(inspector)
add_subdirectory(utils)

qt_add_qml_module(libMozillavpn
    URI Mozilla.VPN.Cpp
    VERSION 1.0
    STATIC
    NO_PLUGIN
    NO_GENERATE_QMLDIR
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Mozilla/VPN/Cpp
)
mz_target_handle_warnings(libMozillavpn)
target_link_libraries(libMozillavpn PRIVATE
    shared-sources
    mozillavpn-sources
)

target_link_libraries(libMozillavpn PUBLIC
    lottie
    mzutils
    nebula
    translations
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Test
    Qt6::WebSockets
    Qt6::Widgets
    Qt6::Svg
)
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten"
   AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(libMozillavpn PUBLIC Qt6::NetworkAuth)
endif()

target_compile_definitions(libMozillavpn PUBLIC
    "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>"
)
if ((${CMAKE_SYSTEM_NAME} STREQUAL "Android") OR (${CMAKE_SYSTEM_NAME} STREQUAL "iOS"))
    target_compile_definitions(libMozillavpn PUBLIC "MZ_MOBILE")
endif()

target_include_directories(libMozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/composer
)

target_include_directories(libMozillavpn PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/addons
    ${CMAKE_CURRENT_BINARY_DIR}
)

include(${CMAKE_SOURCE_DIR}/scripts/cmake/whole-archive.cmake)
link_whole_archive(mozillavpn libMozillavpn)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR
   ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR
   ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_subdirectory(webextension)
    add_subdirectory(interventions)
    target_link_libraries(libMozillavpn PRIVATE mz_webextension)
endif()

target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

include(cmake/${MZ_PLATFORM_NAME}.cmake)


add_subdirectory(ui)
target_link_libraries(mozillavpn PRIVATE mozillavpn-uiplugin)
qt_finalize_target(mozillavpn)
