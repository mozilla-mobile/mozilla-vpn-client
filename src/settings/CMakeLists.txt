# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_settings STATIC)

target_sources(mozillavpn_settings INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settingsholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settingsholder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/settingslist.h
)

if(${MZ_PLATFORM_NAME} STREQUAL "wasm")
    target_sources(mozillavpn_settings INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmcryptosettings.cpp)
elseif(${MZ_PLATFORM_NAME} STREQUAL "macos" OR ${MZ_PLATFORM_NAME} STREQUAL "ios")
    target_sources(mozillavpn_settings INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/cryptosettings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/macos/macoscryptosettings.mm)
elseif(${MZ_PLATFORM_NAME} STREQUAL "android")
    target_sources(mozillavpn_settings INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/android/androidcryptosettings.cpp)
elseif(${MZ_PLATFORM_NAME} STREQUAL "linux")
    target_sources(mozillavpn_settings INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/linux/linuxcryptosettings.cpp)
elseif(${MZ_PLATFORM_NAME} STREQUAL "linux")
    target_sources(mozillavpn_settings INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/windowscryptosettings.cpp)
endif()

target_include_directories(mozillavpn_settings PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_settings PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_settings PUBLIC Qt6::Core)

target_link_libraries(mozillavpn_settings PUBLIC
    mozillavpn_hacl-star
    mozillavpn_logging
    mozillavpn_environment
    mozillavpn_utilities
    mozillavpn_feature
)
