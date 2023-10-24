# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_qmlengineholder STATIC)

target_sources(mozillavpn_qmlengineholder INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qmlengineholder.h
)

target_include_directories(mozillavpn_qmlengineholder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_qmlengineholder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_qmlengineholder PUBLIC Qt6::Core Qt6::Qml)

target_link_libraries(mozillavpn_qmlengineholder PUBLIC
    mozillavpn_utilities
    mozillavpn_networking
)
