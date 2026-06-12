# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

get_filename_component(ADDON_SCRIPT_DIR ${CMAKE_CURRENT_LIST_DIR}/../addon REALPATH)

### Find the "latest update" addon manifest.
#
# Given a list of addon manifest paths, this picks the one whose addon
# directory name contains the word "update" and carries the highest
# vMAJOR.MINOR version (e.g. message_update_v2.38). Manifests with no version
# in the name (like message_update_default) are ignored. The result is
# returned in the caller's scope via OUTVAR (empty string if none match).
function(find_latest_update_addon OUTVAR)
    set(_best_path "")
    set(_best_value -1)
    foreach(_manifest ${ARGN})
        # Match against the addon's directory name, not the full path, so we
        # don't pick up version-looking digits from elsewhere in the path.
        get_filename_component(_dir ${_manifest} DIRECTORY)
        get_filename_component(_name ${_dir} NAME)

        if(NOT _name MATCHES "update")
            continue()
        endif()
        if(NOT _name MATCHES "v([0-9]+)\\.([0-9]+)")
            continue()
        endif()

        # Fold MAJOR.MINOR into a single integer so v2.38 > v2.9 numerically.
        math(EXPR _value "${CMAKE_MATCH_1} * 1000 + ${CMAKE_MATCH_2}")
        if(_value GREATER _best_value)
            set(_best_value ${_value})
            set(_best_path ${_manifest})
        endif()
    endforeach()
    set(${OUTVAR} "${_best_path}" PARENT_SCOPE)
endfunction()

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

    # Fix Ninja dependency tracking when dealing with absolute paths. 
    cmake_policy(PUSH)
    cmake_policy(SET CMP0116 NEW)

    if(NOT ADDON_OUTPUT_DIR)
        set(ADDON_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    else()
        file(MAKE_DIRECTORY ${ADDON_OUTPUT_DIR})
    endif()

    # Get the path to the Qt host tools.
    get_target_property(QT_PATHS_EXECUTABLE Qt6::qtpaths LOCATION)
    get_filename_component(QT_TOOL_BIN_PATH ${QT_PATHS_EXECUTABLE} PATH)
    get_target_property(QT_RCC_EXECUTABLE Qt6::rcc LOCATION)
    get_filename_component(QT_TOOL_LIBEXEC_PATH ${QT_RCC_EXECUTABLE} PATH)

    # Scan for addon manifests if a source directory was provided.
    if(ADDON_SOURCE_DIR)
        file(GLOB_RECURSE SCANNED_MANIFEST_FILES
            CONFIGURE_DEPENDS
            ${ADDON_SOURCE_DIR}/*/manifest.json)

        foreach(FILENAME ${SCANNED_MANIFEST_FILES})
            if(${FILENAME} MATCHES "generated")
                continue()
            endif()
            list(APPEND ADDON_SOURCES ${FILENAME})
        endforeach()
    endif()

    # Prepare some common addon build arguments.
    set(ADDON_BUILD_ARGS -q ${QT_TOOL_BIN_PATH} -q ${QT_TOOL_LIBEXEC_PATH})
    if (ADDON_I18N_DIR)
        list(APPEND ADDON_BUILD_ARGS -i ${ADDON_I18N_DIR})
    endif()

    ## Update default update file (for languages that haven't translated latest bullet points)

    # Get latest update file
    find_latest_update_addon(LATEST_UPDATE_MANIFEST ${ADDON_SOURCES})
    message(STATUS "Latest update addon manifest: ${LATEST_UPDATE_MANIFEST}")

    # Future todo: Ensure that all non-manifest files are identical-ish between latest update message and default update message,
    # to ensure we keep default's js files up-to-date.

    # Get translations.completeness for latest update file
    # Get list of locales that are not equal to 1. If list is empty, append "fake" to it
    add_custom_command(
        OUTPUT ${ADDON_OUTPUT_DIR}/locales_skipped.txt
        DEPENDS ${LATEST_UPDATE_MANIFEST}
        COMMAND ${PYTHON_EXECUTABLE} ${ADDON_SCRIPT_DIR}/translate.py
                    ${ADDON_BUILD_ARGS} ${LATEST_UPDATE_MANIFEST} ${ADDON_OUTPUT_DIR}
    )
    message(STATUS "Skipped locales file: ${ADDON_OUTPUT_DIR}/locales_skipped.txt")

    # Get date, max_client_version, and short_version lines in latest file
    # copy these 4 things to appropriate spot in default file
    add_custom_command(
        OUTPUT ${ADDON_SOURCE_DIR}/message_update_default/manifest.json
        DEPENDS ${ADDON_OUTPUT_DIR}/locales_skipped.txt
        COMMAND ${PYTHON_EXECUTABLE} ${ADDON_SCRIPT_DIR}/adjust_default_update_addon.py
                    ${LATEST_UPDATE_MANIFEST} ${ADDON_SOURCE_DIR}/message_update_default/manifest.json ${ADDON_OUTPUT_DIR}/locales_skipped.txt
    )
    message(STATUS "Default update addon has been adjusted")

    # Add commands to build the addons
    foreach(MANIFEST_FILE ${ADDON_SOURCES})
        # Parse the manifest to get the addon ID.
        file(READ ${MANIFEST_FILE} MANIFEST_JSON)
        string(JSON ADDON_ID GET "${MANIFEST_JSON}" "id")

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
