# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(glean STATIC)

if(NOT MSVC AND NOT IOS)
  target_compile_options(glean PRIVATE -Wall -Werror -Wno-conversion)
endif()

set_target_properties(glean PROPERTIES FOLDER "Libs")
target_link_libraries(glean PRIVATE Qt6::Core Qt6::Qml)

if( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten" AND 
    NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
    target_link_libraries(glean PRIVATE Qt6::Sql)
endif()

target_include_directories(glean PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(glean PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_filename_component(MVPN_SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts ABSOLUTE)

target_sources(glean PRIVATE
    glean.cpp
    glean.h
    glean.qrc
    telemetry/telemetry.qrc
    telemetry/gleansample.h
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/telemetry)
add_custom_command(
    OUTPUT telemetry/gleansample.h
    DEPENDS metrics.yaml
    COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_gleandefs.py
                -o ${CMAKE_CURRENT_BINARY_DIR}/telemetry/gleansample.h
                ${CMAKE_CURRENT_SOURCE_DIR}/metrics.yaml
)

add_custom_command(
    OUTPUT
        telemetry/qmldir
        telemetry/pings.js
        telemetry/sample.js
        telemetry/telemetry.qrc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS metrics.yaml pings.yaml
    COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_glean.py
)
set_source_files_properties(
    telemetry/gleansample.h
    telemetry/qmldir
    telemetry/pings.js
    telemetry/sample.js
    telemetry/telemetry.qrc
    PROPERTIES GENERATED TRUE
)

# Statically link to the SQLite driver, if it exists.
if(TARGET Qt6::QSQLiteDriverPlugin)
    target_link_libraries(glean PUBLIC Qt6::QSQLiteDriverPlugin)
    qt_import_plugins(glean INCLUDE Qt6::QSQLiteDriverPlugin)
endif()
