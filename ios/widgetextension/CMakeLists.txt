# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

enable_language(Swift)

add_executable(widgetextension)
set_target_properties(widgetextension PROPERTIES
    OUTPUT_NAME "MozillaVPNWidgetExtension"
    XCODE_PRODUCT_TYPE com.apple.product-type.app-extension
    BUNDLE_EXTENSION appex
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    MACOSX_BUNDLE_BUNDLE_NAME "MozillaVPNWidgetExtension"
    MACOSX_BUNDLE_BUNDLE_VERSION "${BUILD_ID}"
    MACOSX_BUNDLE_COPYRIGHT "MPL-2.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}.widget-extension"
    MACOSX_BUNDLE_INFO_STRING "MozillaVPNWidgetExtension"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}-${BUILD_ID}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${BUILD_IOS_APP_IDENTIFIER}.widget-extension"
    XCODE_ATTRIBUTE_SWIFT_VERSION "5.0"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_SWIFT_PRECOMPILE_BRIDGING_HEADER "NO"
    XCODE_ATTRIBUTE_APPLICATION_EXTENSION_API_ONLY "YES"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/ToggleWidgetExtension.entitlements
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/../../Frameworks"
    # Do not strip debug symbols on copy
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP "NO"
    XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "NO"
)

find_library(FW_WIDGETKIT WidgetKit REQUIRED)
find_library(FW_SWIFTUI SwiftUI REQUIRED)

target_link_libraries(widgetextension PRIVATE ${FW_WIDGETKIT})
target_link_libraries(widgetextension PRIVATE ${FW_SWIFTUI})

target_compile_options(widgetextension PRIVATE -DGROUP_ID=\"${BUILD_IOS_GROUP_IDENTIFIER}\")
target_compile_options(widgetextension PRIVATE -DNETWORK_EXTENSION=1)

target_sources(widgetextension PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ToggleIntent.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/ToggleWidgetBundle.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/ToggleWidgetControl.swift
    ${CMAKE_SOURCE_DIR}/ios/NETunnelProviderManager+Extension.swift
)
