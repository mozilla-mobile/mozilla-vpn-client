# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    MACOSX_BUNDLE_ICON_FILE "AppIcon"
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
find_library(FW_USER_NOTIFICATIONS UserNotifications)

target_link_libraries(mozillavpn PRIVATE ${FW_SYSTEMCONFIG})
target_link_libraries(mozillavpn PRIVATE ${FW_SERVICEMGMT})
target_link_libraries(mozillavpn PRIVATE ${FW_SECURITY})
target_link_libraries(mozillavpn PRIVATE ${FW_COREWLAN})
target_link_libraries(mozillavpn PRIVATE ${FW_NETWORK})
target_link_libraries(mozillavpn PRIVATE ${FW_USER_NOTIFICATIONS})

# MacOS platform source files
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscontroller.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscryptosettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscryptosettings.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosmenubar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macospingsender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macospingsender.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosstartatbootwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosstartatbootwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macossystemtraynotificationhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macossystemtraynotificationhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosnetworkwatcher.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosnetworkwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosstatusicon.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosstatusicon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosutils.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/xpcdaemonprotocol.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios/iosnetworkwatcher.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios/iosnetworkwatcher.h
)

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/cmake/golang.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos-daemon.cmake)

# Find the SDK root
if(IS_DIRECTORY ${CMAKE_OSX_SYSROOT})
    set(OSX_SDK_PATH ${CMAKE_OSX_SYSROOT})
elseif(CMAKE_OSX_SYSROOT)
    execute_process(OUTPUT_VARIABLE OSX_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path)
else()
    message(FATAL_ERROR "One of CMAKE_OSX_SYSROOT or ENV{SDKROOT} must be defined")
endif()

# Enable Balrog for update support.
target_compile_definitions(mozillavpn PRIVATE MVPN_BALROG)
target_sources(mozillavpn PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/update/balrog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/update/balrog.h
)

# Build the Wireguard Go tunnel
file(GLOB_RECURSE WIREGUARD_GO_DEPS ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/*.go)
set(WIREGUARD_GO_ENV
    GOCACHE=${CMAKE_BINARY_DIR}/go-cache
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
    GOROOT=${GOLANG_GOROOT}
    SDKROOT=${OSX_SDK_PATH}
    GOOS=darwin
    CGO_ENABLED=1
    GO111MODULE=on
    CGO_CFLAGS='-mmacos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
    CGO_LDFLAGS='-mmacos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
)

if(CMAKE_OSX_ARCHITECTURES)
    foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
        string(REPLACE "x86_64" "amd64" GOARCH ${OSXARCH})
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
            COMMENT "Building wireguard-go for ${OSXARCH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
            DEPENDS
                ${WIREGUARD_GO_DEPS}
                ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.mod
                ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.sum
            COMMAND ${CMAKE_COMMAND} -E env ${WIREGUARD_GO_ENV} GOARCH=${GOARCH}
                    ${GOLANG_BUILD_TOOL} build -buildmode exe -buildvcs=false -trimpath -v -ldflags='-s -w'
                        -o ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH}
        )
        list(APPEND WG_GO_ARCH_BUILDS ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go-${OSXARCH})
    endforeach()

    add_custom_target(build_wireguard_go
        COMMENT "Building wireguard-go"
        DEPENDS ${WG_GO_ARCH_BUILDS}
        COMMAND ${LIPO_BUILD_TOOL} -create -output ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go ${WG_GO_ARCH_BUILDS}
    )
else()
    # This only builds for the host architecture.
    add_custom_target(build_wireguard_go
        COMMENT "Building wireguard-go"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go
        DEPENDS
            ${WIREGUARD_GO_DEPS}
            ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.mod
            ${CMAKE_SOURCE_DIR}/3rdparty/wireguard-go/go.sum
        COMMAND ${CMAKE_COMMAND} -E env ${WIREGUARD_GO_ENV}
                ${GOLANG_BUILD_TOOL} build -buildmode exe -buildvcs=false -trimpath -v -ldflags='-s -w'
                    -o ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
    )
endif()
add_dependencies(mozillavpn build_wireguard_go)
osx_bundle_files(mozillavpn
    FILES ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
    DESTINATION Resources/utils
)

# Install the daemon into the bundle.
add_dependencies(mozillavpn daemon)
osx_bundle_files(mozillavpn
    FILES $<TARGET_FILE:daemon>
    DESTINATION Library/LaunchServices
)

# Install the native messaging extensions into the bundle.
add_dependencies(mozillavpn mozillavpnnp)
osx_bundle_files(mozillavpn FILES
    $<TARGET_FILE:mozillavpnnp>
    ${CMAKE_SOURCE_DIR}/extension/manifests/macos/mozillavpn.json
    DESTINATION Resources/utils
)

# Install the background service plist into the bundle.
configure_file(
    ${CMAKE_SOURCE_DIR}/macos/app/service.plist.in
    ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_OSX_APP_IDENTIFIER}.service.plist
)
osx_bundle_files(mozillavpn FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_OSX_APP_IDENTIFIER}.service.plist
    DESTINATION Library/LaunchDaemons
)

# Install the lproj translation files into the bundle.
get_filename_component(I18N_DIR ${CMAKE_SOURCE_DIR}/3rdparty/i18n ABSOLUTE)
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
                    ${CMAKE_SOURCE_DIR}/src/translations/locversion.plist.in
    )
endforeach()

## Install the LoginItems into the bundle.
add_dependencies(mozillavpn loginitem)
add_custom_command(TARGET mozillavpn POST_BUILD
    COMMENT "Bundling LoginItems"
    COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:loginitem>
        $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Library/LoginItems/$<TARGET_PROPERTY:loginitem,OUTPUT_NAME>.app/
)

## Compile and install the app icons into the bundle.
file(READ ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets/AppIcon.appiconset/Contents.json APPICON_CONTENTS_JSON)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets/AppIcon.appiconset/Contents.json)
string(JSON APPICON_IMAGE_COUNT LENGTH ${APPICON_CONTENTS_JSON} "images")
set(APPICON_IMAGE_INDEX 0)
set(APPICON_IMAGE_FILES)
while(APPICON_IMAGE_INDEX LESS APPICON_IMAGE_COUNT)
    string(JSON FILENAME GET ${APPICON_CONTENTS_JSON} "images" ${APPICON_IMAGE_INDEX} "filename")
    list(APPEND APPICON_IMAGE_FILES ${CMAKE_SOURCE_DIR}/macos/app/Images.xcassets/AppIcon.appiconset/${FILENAME})
    math(EXPR APPICON_IMAGE_INDEX "${APPICON_IMAGE_INDEX} + 1")
endwhile()
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns
    DEPENDS ${APPICON_IMAGE_FILES}
    COMMAND ${PYTHON_EXECUTABLE} -m icnsutil c ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns ${APPICON_IMAGE_FILES}
)

target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns PROPERTIES
    GENERATED TRUE
)
set_property(TARGET mozillavpn APPEND PROPERTY RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns)

# Perform codesigning.
osx_embed_provision_profile(mozillavpn)
osx_codesign_target(mozillavpn FORCE)
