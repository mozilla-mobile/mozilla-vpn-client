# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(Swift)

add_executable(networkextension)
set_target_properties(networkextension PROPERTIES
    OUTPUT_NAME "MozillaVPNNetworkExtension"
    XCODE_PRODUCT_TYPE com.apple.product-type.app-extension
    BUNDLE_EXTENSION appex
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "MozillaVPNNetworkExtension"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}.network-extension"
    MACOSX_BUNDLE_INFO_STRING "MozillaVPNNetworkExtension"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}.network-extension"
    XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/WireGuardNetworkExtension-Bridging-Header.h"
    XCODE_ATTRIBUTE_SWIFT_PRECOMPILE_BRIDGING_HEADER "NO"
    XCODE_ATTRIBUTE_APPLICATION_EXTENSION_API_ONLY "YES"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPNNetworkExtension.entitlements
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../../Frameworks"
)

find_library(FW_ASSETS_LIBRARY AssetsLibrary)
find_library(FW_MOBILE_CORE MobileCoreServices)
find_library(FW_UI_KIT UIKit)

target_link_libraries(networkextension PRIVATE ${FW_ASSETS_LIBRARY})
target_link_libraries(networkextension PRIVATE ${FW_MOBILE_CORE})
target_link_libraries(networkextension PRIVATE ${FW_UI_KIT})

target_compile_options(networkextension PRIVATE -DGROUP_ID=\"${BUILD_IOS_GROUP_IDENTIFIER}\")
target_compile_options(networkextension PRIVATE -DNETWORK_EXTENSION=1)

set(WG_APPLE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources)

target_sources(networkextension PRIVATE
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/WireGuardAdapter.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/PacketTunnelSettingsGenerator.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/DNSResolver.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardNetworkExtension/ErrorNotifier.swift
    ${WG_APPLE_SOURCE_DIR}/Shared/Keychain.swift
    ${WG_APPLE_SOURCE_DIR}/Shared/Model/TunnelConfiguration+WgQuickConfig.swift
    ${WG_APPLE_SOURCE_DIR}/Shared/Model/NETunnelProviderProtocol+Extension.swift
    ${WG_APPLE_SOURCE_DIR}/Shared/Model/String+ArrayConversion.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/TunnelConfiguration.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/IPAddressRange.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/Endpoint.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/DNSServer.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/InterfaceConfiguration.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/PeerConfiguration.swift
    ${WG_APPLE_SOURCE_DIR}/Shared/FileManager+Extension.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKitC/x25519.c
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/Array+ConcurrentMap.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/IPAddress+AddrInfo.swift
    ${WG_APPLE_SOURCE_DIR}/WireGuardKit/PrivateKey.swift
    ${CMAKE_SOURCE_DIR}/src/apps/vpn/platforms/ios/iostunnel.swift
    ${CMAKE_SOURCE_DIR}/src/apps/vpn/platforms/ios/iosglue.mm
    ${CMAKE_SOURCE_DIR}/src/apps/vpn/platforms/ios/ioslogger.swift
)

## Build wireguard-go-version.h
execute_process(
    COMMAND go list -m golang.zx2c4.com/wireguard
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKitGo
    OUTPUT_VARIABLE WG_VERSION_FULL
)
string(REGEX REPLACE ".*v\([0-9.]*\).*" "\\1" WG_VERSION_STRING ${WG_VERSION_FULL})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wireguard-go-version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-version.h)
target_sources(networkextension PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-version.h)

target_include_directories(networkextension PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(networkextension PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

## Build the wireguard go library
## TODO: The upstream makefile also makes an attempt to patch the golang runtime
## to provide the boot-time clock instead of an uptime clock. We should probably
## make an attempt to do the same, somehow?
include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)

## Build the wireguard go library for iOS simulation.
## TODO: Some special handling around GOARCH for
execute_process(OUTPUT_VARIABLE SIM_SDKROOT OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND xcrun --sdk iphonesimulator --show-sdk-path)
build_go_archive(${CMAKE_CURRENT_BINARY_DIR}/libwg-sim.a ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKitGo/go.mod
    GOOS ios
    GOARCH amd64
    CGO_CFLAGS -arch x86_64 -isysroot ${SIM_SDKROOT}
    CGO_LDFLAGS -arch x86_64 -isysroot ${SIM_SDKROOT}
)

## Build the wireguard go library for iOS devices.
execute_process(OUTPUT_VARIABLE IOS_SDKROOT OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path)
build_go_archive(${CMAKE_CURRENT_BINARY_DIR}/libwg-dev.a ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKitGo/go.mod
    GOOS ios
    GOARCH arm64
    CGO_CFLAGS -arch arm64 -isysroot ${IOS_SDKROOT}
    CGO_LDFLAGS -arch arm64 -isysroot ${IOS_SDKROOT}
)

## Unify the wireguard go libraries.
add_custom_target(libwg_builder
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/libwg-dev.a
        ${CMAKE_CURRENT_BINARY_DIR}/libwg-sim.a
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/libwg-unified.a
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND lipo -create -output libwg-unified.a libwg-dev.a libwg-sim.a
)
set_target_properties(libwg_builder PROPERTIES FOLDER "Libs")

## Link and depend on the wireguard library.
add_dependencies(networkextension libwg_builder)
target_link_libraries(networkextension PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/libwg-unified.a)

## Add Glean to the extension
target_link_libraries(networkextension PRIVATE iosglean)
set_source_files_properties(
    ${CMAKE_BINARY_DIR}/src/generated/VPNMetrics.swift
    PROPERTIES GENERATED TRUE
)
target_sources(networkextension PRIVATE
    ${CMAKE_BINARY_DIR}/src/generated/VPNMetrics.swift
)
