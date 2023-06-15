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

list(APPEND PINGS_DEP_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/pings_deprecated.yaml)
list(APPEND METRICS_DEP_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/metrics_deprecated.yaml)

get_filename_component(APPS_DIR ${CMAKE_SOURCE_DIR}/src/apps ABSOLUTE)
file(GLOB APPS_NAMES LIST_DIRECTORIES true RELATIVE ${APPS_DIR} ${APPS_DIR}/*)
foreach(APP ${APPS_NAMES})
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
        list(APPEND PINGS_DEP_LIST ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
        list(APPEND METRICS_DEP_LIST ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
    endif()
endforeach()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/telemetry)
add_custom_command(
    OUTPUT telemetry/gleansample.h
    DEPENDS ${METRICS_DEP_LIST}
    COMMAND ${PYTHON_EXECUTABLE} ${MVPN_SCRIPT_DIR}/utils/generate_gleandefs.py
                -o ${CMAKE_CURRENT_BINARY_DIR}/telemetry/gleansample.h
                ${METRICS_DEP_LIST}
)

add_custom_command(
    OUTPUT
        telemetry/qmldir
        telemetry/pings.js
        telemetry/sample.js
        telemetry/telemetry.qrc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${METRICS_DEP_LIST} ${PINGS_DEP_LIST}
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
if(QT_FEATURE_static AND TARGET Qt6::QSQLiteDriverPlugin)
    message("Statically linking to SQLite plugin")
    target_link_libraries(glean PUBLIC Qt6::QSQLiteDriverPlugin)
    qt_import_plugins(glean INCLUDE Qt6::QSQLiteDriverPlugin)
endif()
