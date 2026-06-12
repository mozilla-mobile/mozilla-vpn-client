# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.20)

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)

project(networkextension VERSION 0.1 LANGUAGES C CXX OBJC
        DESCRIPTION "Mozilla VPN Network Extension"
        HOMEPAGE_URL "https://vpn.mozilla.org"
)

# Set some variables when we are building this extension standalone.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OSX version to target for deployment")
    endif()
    if (NOT BUILD_OSX_APP_IDENTIFIER)
        set(BUILD_OSX_APP_IDENTIFIER org.mozilla.macos.FirefoxVPN CACHE STRING "OSX Application identifier")
    endif()
    if(NOT BUILD_VPN_DEVELOPMENT_TEAM)
        set(BUILD_VPN_DEVELOPMENT_TEAM 43AQ936H96 CACHE STRING "Mozilla VPN Development Team")
    endif()
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${BUILD_VPN_DEVELOPMENT_TEAM})
endif()

find_library(FW_FOUNDATION Foundation)
find_library(FW_NW_EXTENSION NetworkExtension)

# The VPN Split Tunnel extension
add_executable(networkextension)
set_target_properties(networkextension PROPERTIES
    OUTPUT_NAME "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    BUNDLE_EXTENSION systemextension
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN Network Extension"
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_PACKAGE_TYPE "SYSX"
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_ENABLE_BITCODE NO
    XCODE_ATTRIBUTE_GCC_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/network-extension.entitlements
)

target_compile_options(networkextension PUBLIC "-fobjc-arc" "-fno-objc-arc-exceptions")
target_compile_definitions(networkextension PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")

# Improve the macOS debug experience - tune for LLDB and copy dSYM into
# the bundle when we are building for the Debug configuration.
target_compile_options(networkextension PRIVATE $<$<CONFIG:Debug>:-O0 -gfull -glldb>)
target_link_options(networkextension PRIVATE $<$<CONFIG:Debug>:-O0 -gfull -glldb>)
add_custom_command(TARGET networkextension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE:networkextension>.dSYM
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,env,true> -- dsymutil $<TARGET_FILE:networkextension> -o $<TARGET_FILE:networkextension>.dSYM
)

target_link_libraries(networkextension PRIVATE ${FW_FOUNDATION})
target_link_libraries(networkextension PRIVATE ${FW_NW_EXTENSION})
target_sources(networkextension PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaceconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaceconfig.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/main.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardpeer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardpeer.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardstatus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardstatus.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardtunnel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/wireguardtunnel.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/VPNSplitTunnelProvider.mm
)

include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)
add_rust_library(boringtun
    PACKAGE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/boringtun
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
    FEATURES ffi-bindings
    CRATE_NAME boringtun
)
target_include_directories(networkextension PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/boringtun/boringtun/src)
target_link_libraries(networkextension PRIVATE boringtun)

# Install an embedded provisioning profile if one exists in the source directory.
if((NOT XCODE) AND (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile))
    target_sources(networkextension PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile PROPERTIES
        MACOSX_PACKAGE_LOCATION "/"
        HEADER_FILE_ONLY TRUE
    )
endif()

# Create a target for the DNS manager tool.
add_executable(dnsmanager ${CMAKE_CURRENT_SOURCE_DIR}/dnsmanager.mm)
find_library(FW_SYSTEMCONFIG SystemConfiguration)
target_compile_options(dnsmanager PUBLIC "-fobjc-arc" "-fno-objc-arc-exceptions")
target_compile_definitions(dnsmanager PRIVATE "$<$<CONFIG:Debug>:MZ_DEBUG>")
target_link_libraries(dnsmanager PRIVATE ${FW_FOUNDATION} ${FW_SYSTEMCONFIG})
osx_codesign_target(dnsmanager)

# Set the DNS manager tool up as a link dependency to ensure the codesign gets updated.
add_dependencies(networkextension dnsmanager)
set_target_properties(networkextension PROPERTIES LINK_DEPENDS $<TARGET_FILE:dnsmanager>)

target_sources(networkextension PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/dnsmanager)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/dnsmanager PROPERTIES
    MACOSX_PACKAGE_LOCATION MacOS
    HEADER_FILE_ONLY TRUE
    GENERATED TRUE
)

osx_codesign_target(networkextension FORCE)
