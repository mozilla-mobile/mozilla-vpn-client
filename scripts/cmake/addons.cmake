# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

### Helper function to build a VPN Addon
#
# This function takes one mandatory argument: ADDON_DIR which provides
# the path to the location where the addon's manifest.json can be found.
#
function(add_addon_target NAME)
    cmake_parse_arguments(ADDON
        ""
        "SOURCE_DIR;OUTPUT_DIR"
        "SOURCES"
        ${ARGN})

    if(NOT ADDON_OUTPUT_DIR)
        set(ADDON_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    # Get the path to the Qt host tools.
    get_target_property(QT_QMLLINT_EXECUTABLE Qt6::qmllint LOCATION)
    get_filename_component(QT_TOOL_PATH ${QT_QMLLINT_EXECUTABLE} PATH)

    # Scan for addon manifests if a source directory was provided.
    if(ADDON_SOURCE_DIR)
        file(GLOB_RECURSE SCANNED_MANIFEST_FILES ${ADDON_SOURCE_DIR}/manifest.json)
        foreach(FILENAME ${SCANNED_MANIFEST_FILES})
            if(${FILENAME} MATCHES "generated")
                continue()
            endif()
            list(APPEND ADDON_SOURCES ${FILENAME})
        endforeach()
    endif()

    # Add commands to build the addons
    foreach(MANIFEST_FILE ${ADDON_SOURCES})
        message("Parsing ${MANIFEST_FILE}...")

        # Parse the manifest to get the addon ID.
        file(READ ${MANIFEST_FILE} ADDON_MANIFEST)
        string(JSON ADDON_ID GET ${ADDON_MANIFEST} "id")

        add_custom_command(
            OUTPUT ${ADDON_OUTPUT_DIR}/${ADDON_ID}.rcc
            DEPENDS ${MANIFEST_FILE}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/addon/build.py
                        -q ${QT_TOOL_PATH} ${MANIFEST_FILE} ${ADDON_OUTPUT_DIR}
        )
        list(APPEND ADDON_OUTPUT_FILES ${ADDON_OUTPUT_DIR}/${ADDON_ID}.rcc)
    endforeach()

    # Add a command to generate the addon index.
    add_custom_command(
        OUTPUT ${ADDON_OUTPUT_DIR}/manifest.json
        DEPENDS ${ADDON_OUTPUT_FILES}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/addon/index.py
                    -o ${ADDON_OUTPUT_DIR}/manifest.json ${ADDON_OUTPUT_FILES}
    )

    # Add a custom target to gather the dependencies together.
    add_custom_target(${NAME} DEPENDS ${ADDON_OUTPUT_FILES} ${ADDON_OUTPUT_DIR}/manifest.json)
endfunction(add_addon_target)
