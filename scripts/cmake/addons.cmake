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
        "SOURCE_DIR;OUTPUT_DIR;I18N_DIR"
        "SOURCES"
        ${ARGN})

    if(${CMAKE_VERSION} VERSION_LESS 3.20)
        message(WARNING "Addon generation requires CMake 3.20 or later")
        return()
    endif()

    # Fix Ninja dependency tracking when dealing with absolute paths. 
    cmake_policy(PUSH)
    cmake_policy(SET CMP0116 NEW)

    if(NOT ADDON_OUTPUT_DIR)
        set(ADDON_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    else()
        file(MAKE_DIRECTORY ${ADDON_OUTPUT_DIR})
    endif()

    # Get the path to the Qt host tools.
    get_target_property(QT_QMLLINT_EXECUTABLE Qt6::qmllint LOCATION)
    get_filename_component(QT_TOOL_BIN_PATH ${QT_QMLLINT_EXECUTABLE} PATH)
    get_target_property(QT_RCC_EXECUTABLE Qt6::rcc LOCATION)
    get_filename_component(QT_TOOL_LIBEXEC_PATH ${QT_RCC_EXECUTABLE} PATH)

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

    # Prepare some common addon build arguments.
    get_filename_component(ADDON_SCRIPT_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../addon REALPATH)
    set(ADDON_BUILD_ARGS -q ${QT_TOOL_BIN_PATH} -q ${QT_TOOL_LIBEXEC_PATH})
    if (ADDON_I18N_DIR)
        list(APPEND ADDON_BUILD_ARGS -i ${ADDON_I18N_DIR})
    endif()

    # Add commands to build the addons
    foreach(MANIFEST_FILE ${ADDON_SOURCES})
        # Parse the manifest to get the addon ID.
        file(READ ${MANIFEST_FILE} ADDON_MANIFEST)
        string(JSON ADDON_ID GET ${ADDON_MANIFEST} "id")

        if((CMAKE_GENERATOR MATCHES "Ninja") OR (CMAKE_GENERATOR MATCHES "Makefiles"))
            ## Depfiles are great, but they only work for some generators.
            file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_deps)
            add_custom_command(
                OUTPUT ${ADDON_OUTPUT_DIR}/${ADDON_ID}.rcc
                DEPENDS ${MANIFEST_FILE}
                DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_deps/${ADDON_ID}.d
                COMMAND ${PYTHON_EXECUTABLE} ${ADDON_SCRIPT_DIR}/build.py
                            -d ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_deps/${ADDON_ID}.d
                            ${ADDON_BUILD_ARGS} ${MANIFEST_FILE} ${ADDON_OUTPUT_DIR}
            )
        else()
            add_custom_command(
                OUTPUT ${ADDON_OUTPUT_DIR}/${ADDON_ID}.rcc
                DEPENDS ${MANIFEST_FILE}
                COMMAND ${PYTHON_EXECUTABLE} ${ADDON_SCRIPT_DIR}/build.py
                            ${ADDON_BUILD_ARGS} ${MANIFEST_FILE} ${ADDON_OUTPUT_DIR}
            )
        endif()
        list(APPEND ADDON_OUTPUT_FILES ${ADDON_OUTPUT_DIR}/${ADDON_ID}.rcc)
    endforeach()

    ## Reset our policy changes
    cmake_policy(POP)

    # Add a command to generate the addon index.
    add_custom_command(
        OUTPUT ${ADDON_OUTPUT_DIR}/manifest.json
        DEPENDS ${ADDON_OUTPUT_FILES}
        COMMAND ${PYTHON_EXECUTABLE} ${ADDON_SCRIPT_DIR}/index.py
                    -o ${ADDON_OUTPUT_DIR}/manifest.json ${ADDON_OUTPUT_FILES}
    )

    # Add a custom target to gather the dependencies together.
    add_custom_target(${NAME} DEPENDS ${ADDON_OUTPUT_FILES} ${ADDON_OUTPUT_DIR}/manifest.json)
endfunction(add_addon_target)
