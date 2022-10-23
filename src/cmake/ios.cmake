# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(OBJC)
enable_language(OBJCXX)
enable_language(Swift)

# Configure the application bundle Info.plist
set_target_properties(mozillavpn PROPERTIES
    OUTPUT_NAME "Mozilla VPN"
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/ios/app/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Mozilla VPN"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}"
    MACOSX_BUNDLE_ICON_FILE "AppIcon"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}"
)
target_include_directories(mozillavpn PRIVATE ${CMAKE_SOURCE_DIR})

get_target_property(QML_GENERATED_IMPORT_DIR mozillavpn QT_QML_MODULE_OUTPUT_DIRECTORY)
message("QML Imports generated at ${QML_GENERATED_IMPORT_DIR}")
file(MAKE_DIRECTORY ${QML_GENERATED_IMPORT_DIR})
qt6_import_qml_plugins(mozillavpn)

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
    platforms/macos/macospingsender.cpp
    platforms/macos/macospingsender.h
    tasks/purchase/taskpurchase.cpp
    tasks/purchase/taskpurchase.h
    platforms/ios/iosiaphandler.mm
    platforms/ios/iosiaphandler.h
    platforms/ios/ioscontroller.mm
    platforms/ios/ioscontroller.h
    platforms/ios/iosdatamigration.mm
    platforms/ios/iosdatamigration.h
    platforms/ios/iosglue.mm
    platforms/ios/iosnetworkwatcher.mm
    platforms/ios/iosnetworkwatcher.h
    platforms/ios/iosnotificationhandler.mm
    platforms/ios/iosnotificationhandler.h
    platforms/ios/iosutils.mm
    platforms/ios/iosutils.h
    platforms/macos/macoscryptosettings.mm
)

# Include the Adjust SDK
#add_definitions(-DMVPN_ADJUST)
#target_sources(mozillavpn PRIVATE
#    adjust/adjustfiltering.cpp
#    adjust/adjustfiltering.h
#    adjust/adjusthandler.cpp
#    adjust/adjusthandler.h
#    adjust/adjustproxy.cpp
#    adjust/adjustproxy.h
#    adjust/adjustproxyconnection.cpp
#    adjust/adjustproxyconnection.h
#    adjust/adjustproxypackagehandler.cpp
#    adjust/adjustproxypackagehandler.h
#    adjust/adjusttasksubmission.cpp
#    adjust/adjusttasksubmission.h
#    platforms/ios/iosadjusthelper.mm
#    platforms/ios/iosadjusthelper.h
#)

set_target_properties(mozillavpn PROPERTIES
    XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_SOURCE_DIR}/ios/app/WireGuard-Bridging-Header.h"
    XCODE_ATTRIBUTE_SWIFT_PRECOMPILE_BRIDGING_HEADER "NO"
    XCODE_ATTRIBUTE_SWIFT_OPTIMIZATION_LEVEL "-Onone"
    XCODE_ATTRIBUTE_SWIFT_OBJC_INTERFACE_HEADER_NAME "Mozilla_VPN-Swift.h"
)
target_compile_options(mozillavpn PRIVATE
    -DGROUP_ID=\"group.org.mozilla.ios.Guardian\"
    -DVPN_NE_BUNDLEID=\"${BUILD_OSX_APP_IDENTIFIER}.network-extension\"
)
target_sources(mozillavpn PRIVATE
    platforms/ios/ioscontroller.swift
    platforms/ios/ioslogger.swift
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

## Build wireguard-go-version.h
execute_process(
    COMMAND go list -m golang.zx2c4.com/wireguard
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-apple/Sources/WireGuardKitGo
    OUTPUT_VARIABLE WG_VERSION_FULL
)
string(REGEX REPLACE ".*v\([0-9.]*\).*" "\\1" WG_VERSION_STRING ${WG_VERSION_FULL})
configure_file(${CMAKE_SOURCE_DIR}/ios/wireguard-go-version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-version.h)
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-version.h)

## Install the Network Exetnsion into the bundle.
add_dependencies(mozillavpn networkextension)
add_custom_command(TARGET mozillavpn POST_BUILD
    COMMENT "Bundling Network Extension"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:networkextension>
        $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/PlugIns/$<TARGET_PROPERTY:networkextension,OUTPUT_NAME>.appex/
)
