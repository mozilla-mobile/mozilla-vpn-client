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
find_library(FW_NW_EXTENSION NetworkExtension)
find_library(FW_SYS_EXTENSION SystemExtensions)

target_link_libraries(mozillavpn PRIVATE ${FW_SYSTEMCONFIG})
target_link_libraries(mozillavpn PRIVATE ${FW_SERVICEMGMT})
target_link_libraries(mozillavpn PRIVATE ${FW_SECURITY})
target_link_libraries(mozillavpn PRIVATE ${FW_COREWLAN})
target_link_libraries(mozillavpn PRIVATE ${FW_NETWORK})
target_link_libraries(mozillavpn PRIVATE ${FW_USER_NOTIFICATIONS})
target_link_libraries(mozillavpn PRIVATE ${FW_NW_EXTENSION})
target_link_libraries(mozillavpn PRIVATE ${FW_SYS_EXTENSION})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macossplittunnelloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macossplittunnelloader.mm
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
    execute_process(OUTPUT_VARIABLE OSX_SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND xcrun --sdk macosx --show-sdk-path)
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
    MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    GOOS=darwin
    CGO_ENABLED=1
    GO111MODULE=on
)
if (CMAKE_CROSSCOMPILING)
    list(APPEND WIREGUARD_GO_ENV
        CGO_CFLAGS='-target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
        CGO_LDFLAGS='-target ${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}'
    )
endif()

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

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        COMMENT "Building wireguard-go"
        DEPENDS ${WG_GO_ARCH_BUILDS}
        COMMAND ${LIPO_BUILD_TOOL} -create -output ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go ${WG_GO_ARCH_BUILDS}
    )
else()
    # This only builds for the host architecture.
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
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

# Manually sign the wireguard-go binary.
if(XCODE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        COMMAND codesign --timestamp -f -s $<QUOTE>$EXPANDED_CODE_SIGN_IDENTITY$<QUOTE> ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        APPEND
    )
elseif(CODE_SIGN_IDENTITY)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        COMMAND ${CODESIGN_BIN} --timestamp -f -s "${CODE_SIGN_IDENTITY}" ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go
        APPEND
    )
endif()

target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/wireguard-go)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/wireguard-go PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/utils
    HEADER_FILE_ONLY TRUE
    GENERATED TRUE
)

# Install the daemon into the bundle.
add_dependencies(mozillavpn daemon)
if(XCODE)
    # We can't use target_sources here as that would require a generator
    # expression to resolve the object file path, and that will break Xcode.
    # Fortunately, Xcode supports embedding targets into the bundle.
    set_target_properties(mozillavpn PROPERTIES
        XCODE_EMBED_XPC_SERVICES daemon
        XCODE_EMBED_XPC_SERVICES_PATH "$(CONTENTS_FOLDER_PATH)/Library/LaunchServices"
    )
else()
    get_target_property(DAEMON_BINARY_DIR daemon BINARY_DIR)
    get_target_property(DAEMON_OUTPUT_NAME daemon OUTPUT_NAME)
    target_sources(mozillavpn PRIVATE ${DAEMON_BINARY_DIR}/${DAEMON_OUTPUT_NAME})
    set_source_files_properties(${DAEMON_BINARY_DIR}/${DAEMON_OUTPUT_NAME} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Library/LaunchServices"
        HEADER_FILE_ONLY TRUE
        GENERATED TRUE
    )
endif()

# Install the background service plist into the bundle.
configure_file(
    ${CMAKE_SOURCE_DIR}/macos/app/xpc-daemon.plist.in
    ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_OSX_APP_IDENTIFIER}.xpc-daemon.plist
)
target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${BUILD_OSX_APP_IDENTIFIER}.xpc-daemon.plist)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${BUILD_OSX_APP_IDENTIFIER}.xpc-daemon.plist PROPERTIES
    MACOSX_PACKAGE_LOCATION Library/LaunchDaemons
    HEADER_FILE_ONLY TRUE
)

# Install the native messaging manifest into the bundle.
target_sources(mozillavpn PRIVATE ${CMAKE_SOURCE_DIR}/extension/manifests/macos/mozillavpn.json)
set_source_files_properties(${CMAKE_SOURCE_DIR}/extension/manifests/macos/mozillavpn.json PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/utils
    HEADER_FILE_ONLY TRUE
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

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lproj/${LOCALE}.lproj)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lproj/${LOCALE}.lproj/locversion.plist
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/translations/locversion.plist.in
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/make_template.py -k LOCALE=${LOCALE}
                    -o ${CMAKE_CURRENT_BINARY_DIR}/lproj/${LOCALE}.lproj/locversion.plist
                    ${CMAKE_CURRENT_SOURCE_DIR}/translations/locversion.plist.in
    )

    target_sources(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/lproj/${LOCALE}.lproj/locversion.plist)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/lproj/${LOCALE}.lproj/locversion.plist PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/${LOCALE}.lproj"
        HEADER_FILE_ONLY TRUE
        GENERATED TRUE
    )
endforeach()

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

# Install the split-tunnel system extension into the bundle.
add_dependencies(mozillavpn split-tunnel)
if(XCODE)
    set_target_properties(mozillavpn PROPERTIES
        XCODE_EMBED_APP_EXTENSIONS split-tunnel
        XCODE_EMBED_APP_EXTENSIONS_PATH "$(CONTENTS_FOLDER_PATH)/Library/SystemExtensions"
    )
else()
    # I can't think of an easy way to do this when the embedded target is a
    # bundle. Using a POST_BUILD hook only runs when the target is relinked
    # and not necessarily when a dependency is rebuilt.
    add_custom_command(TARGET mozillavpn POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Bundling $<TARGET_NAME:split-tunnel>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_BUNDLE_DIR:split-tunnel>
            $<TARGET_BUNDLE_CONTENT_DIR:mozillavpn>/Library/SystemExtensions/$<TARGET_BUNDLE_DIR_NAME:split-tunnel>
    )

    # Create a link dependency to trigger the POST_BUILD hooks in case the split-tunnel
    # gets rebuilt. This only works on Ninja and Makefile generators.
    set_target_properties(mozillavpn PROPERTIES LINK_DEPENDS $<TARGET_FILE:split-tunnel>)
endif()

# Perform codesigning.
osx_embed_provision_profile(mozillavpn)
osx_codesign_target(mozillavpn FORCE)
