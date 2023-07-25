# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

project(addons VERSION 1.0.0 LANGUAGES CXX
        DESCRIPTION "Mozilla VPN Addons"
)

find_program(PYTHON_EXECUTABLE NAMES python3 python)
find_package(Qt6 REQUIRED COMPONENTS Core Qml)

## Create the addons target.
include(${CMAKE_CURRENT_SOURCE_DIR}/../scripts/cmake/addons.cmake)
add_addon_target(addons
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    I18N_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/translations/i18n
)

# If this is the top level project, build and install the addons by default.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(addons PROPERTIES EXCLUDE_FROM_ALL FALSE)
endif()

# Generate the validation executable
add_executable(validation validate.cpp)
target_link_libraries(validation PRIVATE Qt6::Core)

# Validate the generated addons
add_custom_command(
    TARGET addons
    DEPENDS validation
    COMMENT "Validating generated addons"
    COMMAND ${CMAKE_BINARY_DIR}/validation ${CMAKE_BINARY_DIR}
    POST_BUILD
)
