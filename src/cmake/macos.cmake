# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(OBJC)
enable_language(OBJCXX)

set_target_properties(mozillavpn PROPERTIES OUTPUT_NAME "Mozilla VPN")

# Configure the application bundle Info.plist
set_target_properties(mozillavpn PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../macos/app/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Mozilla VPN"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.mozilla.macos.FirefoxVPN"
    MACOSX_BUNDLE_ICON_FILE "AppIcon"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
)

#isEmpty(MVPN_DEVELOPMENT_TEAM) {
#    MVPN_DEVELOPMENT_TEAM = "43AQ936H96"
#}
#
#isEmpty(MVPN_GROUP_ID_MACOS) {
#    MVPN_GROUP_ID_MACOS = "group.org.mozilla.macos.Guardian"
#}
#
#isEmpty(MVPN_APP_ID_PREFIX) {
#    QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.macos
#} else {
#    QMAKE_TARGET_BUNDLE_PREFIX = $$MVPN_APP_ID_PREFIX
#}
#
#QMAKE_BUNDLE = "FirefoxVPN"

# For the loginitem
find_library(FW_SYSTEMCONFIG SystemConfiguration)
find_library(FW_SERVICEMGMT ServiceManagement)
find_library(FW_SECURITY Security)
find_library(FW_COREWLAN CoreWLAN)

target_link_libraries(mozillavpn PRIVATE ${FW_SYSTEMCONFIG})
target_link_libraries(mozillavpn PRIVATE ${FW_SERVICEMGMT})
target_link_libraries(mozillavpn PRIVATE ${FW_SECURITY})
target_link_libraries(mozillavpn PRIVATE ${FW_COREWLAN})

qt6_import_qml_plugins(mozillavpn)

# MacOS platform source files
target_sources(mozillavpn PRIVATE
    daemon/daemon.cpp
    daemon/daemon.h
    daemon/daemonlocalserver.cpp
    daemon/daemonlocalserver.h
    daemon/daemonlocalserverconnection.cpp
    daemon/daemonlocalserverconnection.h
    daemon/dnsutils.h
    daemon/interfaceconfig.h
    daemon/iputils.h
    daemon/wireguardutils.h
    localsocketcontroller.cpp
    localsocketcontroller.h
    platforms/macos/daemon/dnsutilsmacos.cpp
    platforms/macos/daemon/dnsutilsmacos.h
    platforms/macos/daemon/iputilsmacos.cpp
    platforms/macos/daemon/iputilsmacos.h
    platforms/macos/daemon/macosdaemon.cpp
    platforms/macos/daemon/macosdaemon.h
    platforms/macos/daemon/macosdaemonserver.cpp
    platforms/macos/daemon/macosdaemonserver.h
    platforms/macos/daemon/macosroutemonitor.cpp
    platforms/macos/daemon/macosroutemonitor.h
    platforms/macos/daemon/wireguardutilsmacos.cpp
    platforms/macos/daemon/wireguardutilsmacos.h
    platforms/macos/macosauthenticationlistener.cpp
    platforms/macos/macosauthenticationlistener.h
    platforms/macos/macosmenubar.cpp
    platforms/macos/macosmenubar.h
    platforms/macos/macospingsender.cpp
    platforms/macos/macospingsender.h
    platforms/macos/macosstartatbootwatcher.cpp
    platforms/macos/macosstartatbootwatcher.h
    wgquickprocess.cpp
    wgquickprocess.h
    platforms/macos/macoscryptosettings.mm
    platforms/macos/macosnetworkwatcher.mm
    platforms/macos/macosnetworkwatcher.h
    platforms/macos/macosutils.mm
    platforms/macos/macosutils.h
)

include(cmake/golang.cmake)

# Enable Balrog for update support.
add_definitions(-DMVPN_BALROG)
add_go_library(mozillavpn ../balrog/balrog-api.go)
target_sources(mozillavpn PRIVATE
    update/balrog.cpp
    update/balrog.h
)

## A helper to copy files into the application bundle
function(add_bundle_file SOURCE)
    add_custom_command(TARGET mozillavpn POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Bundling ${SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE} $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Resources/utils)
endfunction(add_bundle_file)


# Build the Wireguard Go tunnel
# FIXME: this builds in the source directory.
get_filename_component(WIREGUARD_GO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/wireguard-go ABSOLUTE)
file(GLOB_RECURSE WIREGUARD_GO_DEPS ${WIREGUARD_GO_DIR}/*.go)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/wireguard-go/wireguard-go
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/wireguard-go
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/wireguard-go/main.go
    DEPENDS ${WIREGUARD_GO_DEPS}
    COMMAND make
)
add_bundle_file(${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/wireguard-go/wireguard-go)

# Install the native messaging extensions into the bundle.
add_dependencies(mozillavpn mozillavpnnp)
add_bundle_file($<TARGET_FILE:mozillavpnnp>)
add_bundle_file(${CMAKE_CURRENT_SOURCE_DIR}/../extension/manifests/macos/mozillavpn.json)

#QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
#QMAKE_ASSET_CATALOGS = $$PWD/../../../macos/app/Images.xcassets
#
#LD_RUNPATH_SEARCH_PATHS.name = "LD_RUNPATH_SEARCH_PATHS"
#LD_RUNPATH_SEARCH_PATHS.value = '"$(inherited) @executable_path/../Frameworks"'
#QMAKE_MAC_XCODE_SETTINGS += LD_RUNPATH_SEARCH_PATHS
#
#SWIFT_VERSION.name = "SWIFT_VERSION"
#SWIFT_VERSION.value = "5.0"
#QMAKE_MAC_XCODE_SETTINGS += SWIFT_VERSION
#
#CLANG_ENABLE_MODULES.name = "CLANG_ENABLE_MODULES"
#CLANG_ENABLE_MODULES.value = 'YES'
#QMAKE_MAC_XCODE_SETTINGS += CLANG_ENABLE_MODULES
#
#MARKETING_VERSION.name = "MARKETING_VERSION"
#MARKETING_VERSION.value = $$VERSION
#QMAKE_MAC_XCODE_SETTINGS += MARKETING_VERSION
#
#CODE_SIGN_ENTITLEMENTS.name = "CODE_SIGN_ENTITLEMENTS"
#CODE_SIGN_ENTITLEMENTS.value = $$PWD/../../../macos/app/app.entitlements
#QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_ENTITLEMENTS
#
#CODE_SIGN_IDENTITY.name = "CODE_SIGN_IDENTITY"
#CODE_SIGN_IDENTITY.value = 'Apple Development'
#QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_IDENTITY
#
#DEVELOPMENT_TEAM.name = "DEVELOPMENT_TEAM"
#DEVELOPMENT_TEAM.value = "$$MVPN_DEVELOPMENT_TEAM"
#QMAKE_MAC_XCODE_SETTINGS += DEVELOPMENT_TEAM
#
#GROUP_ID_MACOS.name = "GROUP_ID_MACOS"
#GROUP_ID_MACOS.value = "$$MVPN_GROUP_ID_MACOS"
#QMAKE_MAC_XCODE_SETTINGS += GROUP_ID_MACOS
#
#MVPN_APP_ID_MACOS = "$${QMAKE_TARGET_BUNDLE_PREFIX}.$${QMAKE_BUNDLE}"
#
#APP_ID_MACOS.name = "APP_ID_MACOS"
#APP_ID_MACOS.value = "$$MVPN_APP_ID_MACOS"
#QMAKE_MAC_XCODE_SETTINGS += APP_ID_MACOS
#
#SWIFT_OPTIMIZATION_LEVEL.name = "SWIFT_OPTIMIZATION_LEVEL"
#SWIFT_OPTIMIZATION_LEVEL.value = "-Onone"
#QMAKE_MAC_XCODE_SETTINGS += SWIFT_OPTIMIZATION_LEVEL
#
#GCC_PREPROCESSOR_DEFINITIONS.name = "GCC_PREPROCESSOR_DEFINITIONS"
#GCC_PREPROCESSOR_DEFINITIONS.value = 'GROUP_ID=\"$${MVPN_DEVELOPMENT_TEAM}.$${MVPN_APP_ID_MACOS}\"'
#QMAKE_MAC_XCODE_SETTINGS += GCC_PREPROCESSOR_DEFINITIONS
#