# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.20)

project(split-tunnel VERSION 0.1 LANGUAGES C CXX OBJC
        DESCRIPTION "Mozilla VPN Split Tunnel"
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
add_executable(split-tunnel)
set_target_properties(split-tunnel PROPERTIES
    OUTPUT_NAME "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    BUNDLE_EXTENSION systemextension
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    MACOSX_BUNDLE_INFO_STRING "MozillaVPNSplitTunnel"
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.network-extension"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_PACKAGE_TYPE "SYSX"
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_ENABLE_BITCODE NO
    XCODE_ATTRIBUTE_GCC_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPNSplitTunnel.entitlements
)

target_link_libraries(split-tunnel PRIVATE ${FW_FOUNDATION})
target_link_libraries(split-tunnel PRIVATE ${FW_NW_EXTENSION})
target_sources(split-tunnel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bypasstcpflow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bypasstcpflow.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/main.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/VPNSplitTunnelProvider.mm
)

# Install an embedded provisioning profile if one exists in the source directory.
if((NOT XCODE) AND (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile))
    add_custom_command(TARGET split-tunnel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Bundling split-tunnel embedded.provisionprofile"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile $<TARGET_BUNDLE_CONTENT_DIR:split-tunnel>/embedded.provisionprofile
    )
    target_sources(split-tunnel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile)
    set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/embedded.provisionprofile PROPERTY HEADER_FILE_ONLY TRUE)
endif()

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
osx_codesign_target(split-tunnel FORCE)
