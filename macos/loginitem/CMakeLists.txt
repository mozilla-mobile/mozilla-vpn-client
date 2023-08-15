# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(OBJC)

add_executable(loginitem)

find_library(FW_COCOA Cocoa)
target_link_libraries(loginitem PRIVATE ${FW_COCOA})

set_target_properties(loginitem PROPERTIES
    OUTPUT_NAME "MozillaVPNLoginItem"
    FOLDER "MozillaVPNLoginItem"
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "MozillaVPNLoginItem"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.login-item"
    MACOSX_BUNDLE_ICON_FILE "AppIcon"
    MACOSX_BUNDLE_INFO_STRING "MozillaVPNLoginItem"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/MozillaVPNLoginItem.entitlements"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_OSX_APP_IDENTIFIER}.login-item"
    XCODE_GENERATE_SCHEME TRUE
    # Do not strip debug symbols on copy
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
    XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
)

target_compile_options(loginitem PRIVATE -DAPP_ID="$<TARGET_PROPERTY:mozillavpn,MACOSX_BUNDLE_GUI_IDENTIFIER>")
target_sources(loginitem PRIVATE main.m)

include(${CMAKE_SOURCE_DIR}/scripts/cmake/osxtools.cmake)
osx_codesign_target(loginitem)
