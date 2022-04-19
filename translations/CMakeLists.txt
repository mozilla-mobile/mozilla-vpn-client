# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(translations STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(translations PRIVATE Qt6::Core Qt6::Qml)
target_include_directories(translations PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/generated)

target_sources(translations PRIVATE
    generated/l18nstrings_p.cpp
    generated/l18nstrings.h
    servers.qrc
)

## Generate the string database (language agnostic)
add_custom_command(
    OUTPUT generated/l18nstrings_p.cpp generated/l18nstrings.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    MAIN_DEPENDENCY strings.yaml
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/utils/generate_strings.py
)