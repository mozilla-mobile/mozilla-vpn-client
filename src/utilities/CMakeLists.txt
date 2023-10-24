# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_utilities STATIC)

target_sources(mozillavpn_utilities INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/leakdetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/leakdetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resourceloader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resourceloader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/urlopener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/urlopener.h
    ${CMAKE_CURRENT_SOURCE_DIR}/versionutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/versionutils.h
)

if(${MZ_PLATFORM_NAME} STREQUAL "android")
    target_sources(mozillavpn_utilities INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcommons.h
    )
elseif(${MZ_PLATFORM_NAME} STREQUAL "macos")
    target_sources(mozillavpn_utilities INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosutils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macosutils.mm
    )

    target_link_libraries(mozillavpn_utilities PUBLIC mozillavpn_qmlengineholder)

    find_library(FW_SERVICEMGMT ServiceManagement)
    target_link_libraries(mozillavpn_utilities PUBLIC ${FW_SERVICEMGMT})
elseif(${MZ_PLATFORM_NAME} STREQUAL "windows")
    target_sources(mozillavpn_utilities INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsutils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowsutils.h
    )
elseif(${MZ_PLATFORM_NAME} STREQUAL "ios")
    target_sources(mozillavpn_utilities INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios/ioscommons.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios/ioscommons.mm
    )

    target_link_libraries(mozillavpn_utilities PUBLIC mozillavpn_theme)
endif()

target_include_directories(mozillavpn_utilities PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_utilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_utilities PUBLIC Qt6::Core Qt6::Gui Qt6::Qml)

target_link_libraries(mozillavpn_utilities PUBLIC
    mozillavpn_theme
    mozillavpn_settings
    mozillavpn_logging
    mozillavpn_environment
)
