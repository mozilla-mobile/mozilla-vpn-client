# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(glean STATIC)

find_package(Qt6 REQUIRED COMPONENTS Core Qml)
target_link_libraries(glean PRIVATE Qt6::Core Qt6::Qml)
target_include_directories(glean PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(glean PRIVATE
    glean.cpp
    glean.h
    glean.qrc
    telemetry/gleansample.h
)

set_source_files_properties(telemetry/gleansample.h PROPERTIES GENERATED 1)

add_custom_command(
    OUTPUT telemetry/gleansample.h telemetry/qmldir telemetry/pings.js telemetry/sample.js
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    MAIN_DEPENDENCY metrics.yaml
    DEPENDS pings.yaml
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/utils/generate_glean.py
)

## FIXME: This will fail on wasm
qt6_import_plugins(glean INCLUDE qsqlite)
