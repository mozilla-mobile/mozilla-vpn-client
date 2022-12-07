# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(OBJC)
enable_language(OBJCXX)

set_target_properties(mozillavpn PROPERTIES OUTPUT_NAME "Mozilla VPN")

# Configure the application bundle Info.plist
set_target_properties(mozillavpn PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/app/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "Mozilla VPN"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_SOURCE_DIR}/macos/app/app.entitlements
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}"
    XCODE_ATTRIBUTE_MARKETING_VERSION "${CMAKE_PROJECT_VERSION}"
    XCODE_GENERATE_SCHEME TRUE
)

find_library(FW_SYSTEMCONFIG SystemConfiguration)
find_library(FW_SERVICEMGMT ServiceManagement)
find_library(FW_SECURITY Security)
find_library(FW_COREWLAN CoreWLAN)
find_library(FW_NETWORK Network)

target_link_libraries(mozillavpn PRIVATE ${FW_SYSTEMCONFIG})
target_link_libraries(mozillavpn PRIVATE ${FW_SERVICEMGMT})
target_link_libraries(mozillavpn PRIVATE ${FW_SECURITY})
target_link_libraries(mozillavpn PRIVATE ${FW_COREWLAN})
target_link_libraries(mozillavpn PRIVATE ${FW_NETWORK})

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
    platforms/macos/macossystemtraynotificationhandler.cpp
    platforms/macos/macossystemtraynotificationhandler.h
    wgquickprocess.cpp
    wgquickprocess.h
    platforms/macos/macoscryptosettings.mm
    platforms/macos/macosnetworkwatcher.mm
    platforms/macos/macosnetworkwatcher.h
    platforms/macos/macosstatusicon.mm
    platforms/macos/macosstatusicon.h
    platforms/macos/macosutils.mm
    platforms/macos/macosutils.h
    platforms/ios/iosnetworkwatcher.mm
    platforms/ios/iosnetworkwatcher.h
)

include(cmake/osxtools.cmake)
include(cmake/golang.cmake)
include(cmake/signature.cmake)

# Enable Balrog for update support.
target_compile_definitions(mozillavpn PRIVATE MVPN_BALROG)
add_go_library(balrog-api ../balrog/balrog-api.go
    CGO_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
target_link_libraries(mozillavpn PRIVATE balrog-api)
target_sources(mozillavpn PRIVATE
    update/balrog.cpp
    update/balrog.h
)

# Build the Wireguard Go tunnel
# FIXME: this builds in the source directory.
get_filename_component(WIREGUARD_GO_DIR ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go ABSOLUTE)
file(GLOB_RECURSE WIREGUARD_GO_DEPS ${WIREGUARD_GO_DIR}/*.go)
add_custom_target(build_wireguard_go
    COMMENT "Building wireguard-go"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
    DEPENDS ${WIREGUARD_GO_DEPS}
    COMMAND make
)
add_dependencies(mozillavpn build_wireguard_go)
osx_bundle_files(mozillavpn
    FILES ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/wireguard-go
    DESTINATION Resources/utils
)

# Install the native messaging extensions into the bundle.
add_dependencies(mozillavpn mozillavpnnp)
osx_bundle_files(mozillavpn FILES
    $<TARGET_FILE:mozillavpnnp>
    ${CMAKE_SOURCE_DIR}/extension/manifests/macos/mozillavpn.json
    DESTINATION Resources/utils
)

# Install the lproj translation files into the bundle.
get_filename_component(I18N_DIR ${CMAKE_SOURCE_DIR}/i18n ABSOLUTE)
file(GLOB I18N_LOCALES LIST_DIRECTORIES true RELATIVE ${I18N_DIR} ${I18N_DIR}/*)
list(FILTER I18N_LOCALES EXCLUDE REGEX "^\\..+")
foreach(LOCALE ${I18N_LOCALES})
    if(NOT EXISTS ${I18N_DIR}/${LOCALE}/mozillavpn.xliff)
        continue()
    endif()
    execute_process(
        RESULT_VARIABLE I18N_CHECK_RESULT
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/xlifftool.py --locale=${LOCALE} -c ${I18N_DIR}/${LOCALE}/mozillavpn.xliff
    )
    if(NOT I18N_CHECK_RESULT EQUAL 0)
        continue()
    endif()
    
    add_custom_command(TARGET mozillavpn POST_BUILD
        COMMENT "Bundling locale ${LOCALE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Resources/${LOCALE}.lproj
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/make_template.py -k LOCALE=${LOCALE} 
                    -o $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Resources/${LOCALE}.lproj/locversion.plist
                    ${CMAKE_SOURCE_DIR}/translations/locversion.plist.in
    )
endforeach()

## Install the LoginItems into the bundle.
add_dependencies(mozillavpn loginitem)
add_custom_command(TARGET mozillavpn POST_BUILD
    COMMENT "Bundling LoginItems"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:loginitem>
        $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Library/LoginItems/$<TARGET_PROPERTY:loginitem,OUTPUT_NAME>.app/
)

## Compile and install the asset catalog into the bundle.
osx_bundle_assetcatalog(mozillavpn CATALOG ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets)

# Perform codesigning.
osx_codesign_target(mozillavpn FORCE)

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
#SWIFT_OPTIMIZATION_LEVEL.name = "SWIFT_OPTIMIZATION_LEVEL"
#SWIFT_OPTIMIZATION_LEVEL.value = "-Onone"
#QMAKE_MAC_XCODE_SETTINGS += SWIFT_OPTIMIZATION_LEVEL
