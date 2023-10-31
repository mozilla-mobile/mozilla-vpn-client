# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_networking STATIC)

target_sources(mozillavpn_networking INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/networkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networkmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/simplenetworkmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/simplenetworkmanager.h
)

if (${MZ_PLATFORM_NAME} STREQUAL "wasm")
    target_sources(mozillavpn_networking INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkrequest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/wasm/wasmnetworkrequest.h
    )
else()
    target_sources(mozillavpn_networking INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/networkrequest.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/networkrequest.h
    )
endif()

target_include_directories(mozillavpn_networking PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_networking PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_networking PUBLIC Qt6::Core Qt6::Network)

target_link_libraries(mozillavpn_networking PUBLIC
    mozillavpn_environment
    mozillavpn_settings
    mozillavpn_taskscheduler
    mozillavpn_utilities
)
