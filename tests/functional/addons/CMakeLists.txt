# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.20)

project(testing_addons VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Mozilla VPN Addons for Functional Testing"
)

find_program(PYTHON_EXECUTABLE NAMES python3 python)
find_package(Qt6 REQUIRED COMPONENTS Core Qml)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/cmake/addons.cmake)

# Build the production addons.
add_addon_target(prod_addon
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/prod/
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../addons/
    I18N_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../3rdparty/i18n
)
set_target_properties(prod_addon PROPERTIES EXCLUDE_FROM_ALL FALSE)

# Build the addons for testing.
add_custom_target(test_01_empty_manifest
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/01_empty_manifest
                ${CMAKE_CURRENT_BINARY_DIR}/01_empty_manifest
)
set_target_properties(test_01_empty_manifest PROPERTIES EXCLUDE_FROM_ALL FALSE)

add_custom_target(test_02_broken_manifest
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/02_broken_manifest/
                ${CMAKE_CURRENT_BINARY_DIR}/02_broken_manifest/
)
set_target_properties(test_02_broken_manifest PROPERTIES EXCLUDE_FROM_ALL FALSE)

add_addon_target(test_03_single_addon
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/03_single_addon
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/03_single_addon
)
set_target_properties(test_03_single_addon PROPERTIES EXCLUDE_FROM_ALL FALSE)

add_addon_target(test_08_message_disabled
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/08_message_disabled
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/08_message_disabled
)
set_target_properties(test_08_message_disabled PROPERTIES EXCLUDE_FROM_ALL FALSE)
