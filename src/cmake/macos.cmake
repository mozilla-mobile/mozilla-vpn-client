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
    MACOSX_BUNDLE_ICON_FILE "AppIcon"
    MACOSX_BUNDLE_INFO_STRING "Mozilla VPN"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}"
)

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

include(cmake/osxtools.cmake)
include(cmake/golang.cmake)
include(cmake/signature.cmake)

# Enable Balrog for update support.
add_definitions(-DMVPN_BALROG)
add_go_library(balrog ../balrog/balrog-api.go
    CGO_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
target_link_libraries(mozillavpn PRIVATE balrog)
target_sources(mozillavpn PRIVATE
    update/balrog.cpp
    update/balrog.h
)

# Perform codesigning.
execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/make_template.py ${CMAKE_SOURCE_DIR}/macos/app/app.entitlements
        -k "\$(DEVELOPMENT_TEAM)=${BUILD_OSX_DEVELOPMENT_TEAM}"
        -k "\$(APP_ID_MACOS)=${BUILD_OSX_APP_IDENTIFIER}"
        -o ${CMAKE_CURRENT_BINARY_DIR}/app.entitlements
)
osx_codesign_target(mozillavpn FORCE ENTITLEMENTS ${CMAKE_CURRENT_BINARY_DIR}/app.entitlements)

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
osx_codesign_target(build_wireguard_go FORCE OPTIONS runtime
    FILES ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/wireguard-go
)
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
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/make_template.py -k __LOCALE__=${LOCALE} 
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
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Assets.car
           ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/ac_generated_info.plist
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets/Contents.json
    COMMAND actool --output-format human-readable-text --notices --warnings
                --target-device mac --platform macosx --minimum-deployment-target ${CMAKE_OSX_DEPLOYMENT_TARGET}
                --app-icon AppIcon --output-partial-info-plist ${CMAKE_CURRENT_BINARY_DIR}/ac_generated_info.plist
                --development-region en --enable-on-demand-resources NO
                --compile ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets
)
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/Assets.car
    ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns
)
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/Assets.car
    ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns
    PROPERTIES
    HEADER_FILE_ONLY TRUE
    MACOSX_PACKAGE_LOCATION Resources
)

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
