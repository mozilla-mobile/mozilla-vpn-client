# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(mozillavpn MANUAL_FINALIZATION)

if(NOT MSVC AND NOT IOS)
  target_compile_options(mozillavpn PRIVATE -Wall -Werror -Wno-conversion)
endif()

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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/environment)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/feature)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/hacl-star)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logging)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/networking)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/settings)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/theme)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utilities)
target_link_libraries(mozillavpn PRIVATE
    mozillavpn_environment
    mozillavpn_feature
    mozillavpn_hacl-star
    mozillavpn_logging
    mozillavpn_networking
    mozillavpn_qmlengineholder
    mozillavpn_settings
    mozillavpn_taskscheduler
    mozillavpn_theme
    mozillavpn_utilities
)

target_link_libraries(mozillavpn PRIVATE
    shared-sources
    mozillavpn-sources
    lottie
    nebula
    translations
    qtglean
)

target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

include(cmake/${MZ_PLATFORM_NAME}.cmake)

if(NOT CMAKE_CROSSCOMPILING)
    target_compile_definitions(mozillavpn PUBLIC MVPN_WEBEXTENSION)
endif()

qt6_add_qml_module(mozillavpn
  URI Mozilla.Shared.qmlcomponents
  VERSION 1.0
)

qt_finalize_target(mozillavpn)
