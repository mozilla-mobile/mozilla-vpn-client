# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_environment STATIC)

target_sources(mozillavpn_environment INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/env.h
)

target_include_directories(mozillavpn_environment PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_include_directories(mozillavpn_environment PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_environment PRIVATE Qt6::Core)

target_link_libraries(mozillavpn_environment PUBLIC
    mozillavpn_settings
)
