# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)

# Configure the application bundle Info.plist
set_target_properties(mozillavpn PROPERTIES
    OUTPUT_NAME "Mozilla VPN"
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/ios/app/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Mozilla VPN"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_SOURCE_DIR}/ios/app/main.entitlements"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${CMAKE_PROJECT_VERSION}"
    XCODE_GENERATE_SCHEME TRUE
)
target_include_directories(mozillavpn PRIVATE ${CMAKE_SOURCE_DIR})

find_library(FW_UI_KIT UIKit)
find_library(FW_FOUNDATION Foundation)
find_library(FW_STORE_KIT StoreKit)
find_library(FW_USER_NOTIFICATIONS UserNotifications)
find_library(FW_NETWORK Network)

target_link_libraries(mozillavpn PRIVATE ${FW_UI_KIT})
target_link_libraries(mozillavpn PRIVATE ${FW_FOUNDATION})
target_link_libraries(mozillavpn PRIVATE ${FW_STORE_KIT})
target_link_libraries(mozillavpn PRIVATE ${FW_USER_NOTIFICATIONS})
target_link_libraries(mozillavpn PRIVATE ${FW_NETWORK})

## Hack: IOSUtils needs QtGui internals...
target_include_directories(mozillavpn PRIVATE ${Qt6Gui_PRIVATE_INCLUDE_DIRS})

# iOS platform source files
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/macos/macospingsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/macos/macospingsender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/tasks/purchase/taskpurchase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosiaphandler.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosiaphandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/ioscontroller.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/ioscontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosglue.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosnetworkwatcher.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosnetworkwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosnotificationhandler.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosnotificationhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosutils.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosutils.h
)

# Include the Adjust SDK
if(BUILD_ADJUST_SDK_TOKEN)
    add_compile_definitions(MVPN_ADJUST)
    target_compile_options(mozillavpn PRIVATE -DADJUST_SDK_TOKEN=${BUILD_ADJUST_SDK_TOKEN})
    target_sources(mozillavpn PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustfiltering.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjusthandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjusthandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxyconnection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxyconnection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjustproxypackagehandler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjusttasksubmission.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/adjust/adjusttasksubmission.h
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosadjusthelper.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/iosadjusthelper.h
    )
    target_link_libraries(mozillavpn PRIVATE adjust)
endif()

target_sources(mozillavpn PRIVATE
    ${CMAKE_SOURCE_DIR}/ios/app/launch.png
    ${CMAKE_SOURCE_DIR}/ios/app/MozillaVPNLaunchScreen.storyboard)
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/ios/app/launch.png
    ${CMAKE_SOURCE_DIR}/ios/app/MozillaVPNLaunchScreen.storyboard
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

## Compile and install the asset catalog into the bundle.
osx_bundle_assetcatalog(mozillavpn
    DEVICES iphone ipad
    CATALOG ${CMAKE_SOURCE_DIR}/ios/app/Images.xcassets)

set_target_properties(mozillavpn PROPERTIES
    XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_SOURCE_DIR}/ios/app/WireGuard-Bridging-Header.h"
    XCODE_ATTRIBUTE_SWIFT_PRECOMPILE_BRIDGING_HEADER "NO"
    XCODE_ATTRIBUTE_SWIFT_OPTIMIZATION_LEVEL "-Onone"
    XCODE_ATTRIBUTE_SWIFT_OBJC_INTERFACE_HEADER_NAME "Mozilla_VPN-Swift.h"
)
target_compile_options(mozillavpn PRIVATE
    -DGROUP_ID=\"${BUILD_IOS_GROUP_IDENTIFIER}\"
    -DVPN_NE_BUNDLEID=\"${BUILD_IOS_APP_IDENTIFIER}.network-extension\"
)
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/ioscontroller.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/vpn/platforms/ios/ioslogger.swift
)

target_sources(mozillavpn PRIVATE
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/Shared/Keychain.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/IPAddressRange.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/InterfaceConfiguration.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/Shared/Model/NETunnelProviderProtocol+Extension.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/TunnelConfiguration.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/Shared/Model/TunnelConfiguration+WgQuickConfig.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/Endpoint.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/Shared/Model/String+ArrayConversion.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/PeerConfiguration.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/DNSServer.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardApp/LocalizationHelper.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/Shared/FileManager+Extension.swift
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKitC/x25519.c
    ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKit/PrivateKey.swift
)

## Install the Network Extension into the bundle.
add_dependencies(mozillavpn networkextension)
set_target_properties(mozillavpn PROPERTIES XCODE_EMBED_APP_EXTENSIONS networkextension)

## Install the Glean iOS SDK into the bundle.
include(${CMAKE_SOURCE_DIR}/qtglean/ios.cmake)
add_dependencies(mozillavpn iosglean)
set_target_properties(mozillavpn PROPERTIES XCODE_EMBED_APP_EXTENSIONS iosglean)
