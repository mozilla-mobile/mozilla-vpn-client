# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_feature STATIC)

target_sources(mozillavpn_feature INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/feature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/feature.h
    ${CMAKE_CURRENT_SOURCE_DIR}/featurelist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/featurelistcallback.h
)

target_include_directories(mozillavpn_feature PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_include_directories(mozillavpn_feature PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_feature PUBLIC Qt6::Core Qt6::Widgets)

target_link_libraries(mozillavpn_feature PUBLIC
    mozillavpn_settings
    mozillavpn_logging
    mozillavpn_environment
    mozillavpn_utilities
)
