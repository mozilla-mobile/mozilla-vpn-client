# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(mozillavpn_taskscheduler STATIC)

target_sources(mozillavpn_taskscheduler INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/taskscheduler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/task.h
)

target_include_directories(mozillavpn_taskscheduler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

# By making this public, any target which links to this target
# will add this directory to their include directories list.
target_include_directories(mozillavpn_taskscheduler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mozillavpn_taskscheduler PUBLIC Qt6::Core)

target_link_libraries(mozillavpn_taskscheduler PUBLIC
    mozillavpn_logging
    mozillavpn_utilities
)
