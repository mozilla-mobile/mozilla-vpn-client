# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# For CMake versions prior to 3.19, we may need to perform manual
# finalization in order to pick up QML dependencies when built
# statically.
qt_add_executable(mozillavpn EXCLUDE_FROM_ALL MANUAL_FINALIZATION)

add_custom_target(vpn)
add_dependencies(vpn mozillavpn)

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

if(APPLE)
    ## Setup some defaults for code signing.
    if(NOT BUILD_OSX_APP_IDENTIFIER)
        set(BUILD_OSX_APP_IDENTIFIER org.mozilla.macos.FirefoxVPN CACHE STRING "OSX Application identifier")
    endif()
    if(NOT BUILD_IOS_APP_IDENTIFIER)
        set(BUILD_IOS_APP_IDENTIFIER org.mozilla.ios.FirefoxVPN CACHE STRING "iOS Application identifier")
    endif()
    if(NOT BUILD_IOS_GROUP_IDENTIFIER)
        set(BUILD_IOS_GROUP_IDENTIFIER group.org.mozilla.ios.Guardian CACHE STRING "iOS Group identifier")
    endif()
    if(NOT BUILD_VPN_DEVELOPMENT_TEAM)
        set(BUILD_VPN_DEVELOPMENT_TEAM 43AQ936H96 CACHE STRING "Mozilla VPN Development Team")
    endif()
    set(CMAKE_XCODE_GENERATE_SCHEME FALSE)
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${BUILD_VPN_DEVELOPMENT_TEAM})
    set(CMAKE_XCODE_ATTRIBUTE_GROUP_ID_IOS ${BUILD_IOS_GROUP_IDENTIFIER})
endif()

include(${CMAKE_SOURCE_DIR}/src/shared/sources.cmake)
include(cmake/sources.cmake)
include(cmake/${MZ_PLATFORM_NAME}.cmake)

target_link_libraries(mozillavpn PRIVATE
    glean
    lottie
    mozillavpn-sources
    nebula
    qtglean
    shared-sources
    translations_vpn
)

target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

target_compile_definitions(mozillavpn PRIVATE "MZ_$<UPPER_CASE:${MZ_PLATFORM_NAME}>")
target_compile_definitions(mozillavpn PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")

if(NOT CMAKE_CROSSCOMPILING)
    target_compile_definitions(mozillavpn PUBLIC MVPN_WEBEXTENSION)
endif()

qt6_add_qml_module(mozillavpn
  URI Mozilla.Shared.qmlcomponents
  VERSION 1.0
)

qt_finalize_target(mozillavpn)
