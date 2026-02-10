# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(NOT APPLE)
    message(FATAL_ERROR "OSX Tools are only supported on Apple targets")
endif()

if(CMAKE_COLOR_MAKEFILE)
    set(COMMENT_ECHO_COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --blue --bold)
else()
    set(COMMENT_ECHO_COMMAND ${CMAKE_COMMAND} -E echo)
endif()

if(CODE_SIGN_IDENTITY)
    # When using a conda environment, there is a codesign tool that gets
    # included as a dependency and we really do not want to use it for signing.
    # So strip out the conda environment path when searching for the codesign
    # tool.
    string(REPLACE ":" ";" CODESIGN_PATHS $ENV{PATH})
    if(DEFINED ENV{CONDA_PREFIX})
        foreach(PATH ${CODESIGN_PATHS})
            string(FIND ${PATH} $ENV{CONDA_PREFIX} IS_CONDA_PREFIX)
            if(IS_CONDA_PREFIX EQUAL 0)
                list(REMOVE_ITEM CODESIGN_PATHS ${PATH})
            endif()
        endforeach()
    endif()

    find_program(CODESIGN_BIN
        NAMES codesign
        NO_SYSTEM_ENVIRONMENT_PATH
        PATHS ${CODESIGN_PATHS}
    )
    if(NOT CODESIGN_BIN)
        messsage(FATAL_ERROR "Cannot sign code, could not find 'codesign' executable")
    endif()
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY})
endif()

## A helper to copy files into the application bundle
function(osx_bundle_files TARGET)
    cmake_parse_arguments(BUNDLE
        ""
        "DESTINATION"
        "FILES"
        ${ARGN})
    
    if(NOT BUNDLE_DESTINATION)
        set(BUNDLE_DESTINATION Resources)
    endif()

    foreach(FILE ${BUNDLE_FILES})
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND_EXPAND_LISTS
            COMMAND ${COMMENT_ECHO_COMMAND} "Bundling ${FILE}"
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/${BUNDLE_DESTINATION}
            COMMAND ${CMAKE_COMMAND} -E copy ${FILE} $<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/${BUNDLE_DESTINATION}/
        )
    endforeach()
endfunction(osx_bundle_files)

## A helper to code-sign an executable.
function(osx_codesign_target TARGET)
    ## Xcode should perform automatic code-signing for us.
    if(XCODE)
        return()
    endif()
    ## Not required unless we are configured for codesigning.
    if(NOT CODE_SIGN_IDENTITY)
        return()
    endif()

    cmake_parse_arguments(CODESIGN
        "FORCE"
        ""
        "OPTIONS;FILES"
        ${ARGN})

    set(CODESIGN_ARGS --timestamp -s "${CODE_SIGN_IDENTITY}")
    if(CODESIGN_FORCE)
        list(APPEND CODESIGN_ARGS -f)
    endif()
    if(CODESIGN_OPTIONS)
        list(JOIN CODESIGN_OPTIONS , CODESIGN_OPTIONS_JOINED)
        list(APPEND CODESIGN_ARGS "--option=${CODESIGN_OPTIONS_JOINED}")
    endif()

    ## Process the entitlements as though Xcode has done variable expansion.
    ## This only supports the PRODUCT_BUNDLE_IDENTIFIER and DEVELOPMENT_TEAM
    ## for now.
    get_target_property(CODESIGN_ENTITLEMENTS ${TARGET} XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS)
    if(CODESIGN_ENTITLEMENTS)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/utils/make_template.py ${CODESIGN_ENTITLEMENTS}
                -k PRODUCT_BUNDLE_IDENTIFIER=$<TARGET_PROPERTY:${TARGET},XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER>
                -k DEVELOPMENT_TEAM=${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
                -o ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_codesign.entitlements
        )
        list(APPEND CODESIGN_ARGS --entitlements ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_codesign.entitlements)
    endif()

    ## If no files were specified, sign the target itself.
    if(NOT CODESIGN_FILES)
        set(CODESIGN_FILES $<TARGET_FILE:${TARGET}>)
    endif()

    foreach(FILE ${CODESIGN_FILES})
        add_custom_command(TARGET ${TARGET} POST_BUILD VERBATIM
            COMMAND ${COMMENT_ECHO_COMMAND} "Signing ${TARGET}: ${FILE}"
            COMMAND ${CODESIGN_BIN} ${CODESIGN_ARGS} ${FILE}
        )
    endforeach()
endfunction()

function(osx_embed_provision_profile TARGET)
    ## Xcode should perform manage this for us.
    if(XCODE)
        return()
    endif()
    ## Not required unless we are configured for codesigning.
    if(NOT CODE_SIGN_IDENTITY)
        return()
    endif()
    ## A provisioning profile can be manually specified
    if(CODE_SIGN_PROFILE)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${COMMENT_ECHO_COMMAND} "Bundling embedded.provisionprofile"
            COMMAND ${CMAKE_COMMAND} -E copy ${CODE_SIGN_PROFILE} $<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/embedded.provisionprofile
        )
        return()
    endif()

    # Enumerate the provioning profiles managed by Xcode
    set(XCODE_PROVISION_PROFILES_DIR "$ENV{HOME}/Library/Developer/Xcode/UserData/Provisioning\ Profiles")
    execute_process(
        OUTPUT_VARIABLE XCODE_PROVISION_PROFILES_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND find ${XCODE_PROVISION_PROFILES_DIR} -type f -name "*.provisionprofile"
    )
    string(REGEX MATCHALL "[^\n\r]+" XCODE_PROVISION_PROFILES "${XCODE_PROVISION_PROFILES_RAW}")

    # An inline python script to convert ISO timestamps into UTC seconds.
    # Note: this constructs a CMake list, which uses semicolons for separators,
    # which is perfect because the semicolon also serves to separate statements
    # in python.
    set(PYTHON_ISOCONVERT_SCRIPT
        "import sys"
        "from datetime import datetime"
        "isotime = sys.stdin.readline().strip()"
        "print(datetime.strptime(isotime, '%Y-%m-%dT%H:%M:%S%z').timestamp())"
    )

    # Find the profile which matches our team identifier and has the most recent creation date.
    set(BEST_TIMESTAMP 0)
    set(BEST_PROFILE "")
    foreach(FILENAME ${XCODE_PROVISION_PROFILES})
        # When auto-detecting a provisioning profile, only consider those
        # that provision all devices within the development team.
        execute_process(
            OUTPUT_VARIABLE DEVICES_ALL
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE DEVICES_ALL_RETURN_CODE
            ERROR_QUIET
            COMMAND security cms -D -i ${FILENAME}
            COMMAND plutil -extract ProvisionsAllDevices raw -
        )
        if(NOT DEVICES_ALL_RETURN_CODE EQUAL 0)
            continue()
        elseif(NOT DEVICES_ALL STREQUAL "true")
            continue()
        endif()

        # Extract the creation date of this profile, we will want to use the
        # most-recently created profile that matches the application being
        # signed.
        execute_process(
            OUTPUT_VARIABLE FILE_TIMESTAMP
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE TIMESTAMP_RETURN_CODE
            COMMAND security cms -D -i ${FILENAME}
            COMMAND plutil -extract CreationDate raw -
            COMMAND ${PYTHON_EXECUTABLE} -c "${PYTHON_ISOCONVERT_SCRIPT}"
        )
        if(NOT TIMESTAMP_RETURN_CODE EQUAL 0)
            continue()
        endif()
        if(FILE_TIMESTAMP LESS BEST_TIMESTAMP)
            continue()
        endif()

        # Extract the entitlements from the provisioning profile
        execute_process(
            OUTPUT_VARIABLE ENTITLEMENTS_JSON
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE PLUTIL_RETURN_CODE
            COMMAND security cms -D -i ${FILENAME}
            COMMAND plutil -extract Entitlements xml1 -o - -
            COMMAND plutil -convert json -o - -
        )
        if(NOT PLUTIL_RETURN_CODE EQUAL 0)
            continue()
        endif()

        # The provisioning profile must grant a com.apple.application-identifier
        # entitlement matching the team and bundle identifiers being signed.
        string(JSON PROFILE_APP_IDENTIFIER GET "${ENTITLEMENTS_JSON}" "com.apple.application-identifier")
        get_target_property(TARGET_APP_IDENTIFIER ${TARGET} XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER)
        if(PROFILE_APP_IDENTIFIER STREQUAL "${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}.${TARGET_APP_IDENTIFIER}")
            set(BEST_TIMESTAMP ${FILE_TIMESTAMP})
            set(BEST_PROFILE ${FILENAME})
        endif()
    endforeach()

    # If a provisioning profile was found - embed it into the bundle
    if(BEST_PROFILE)
        # For diagnostic assistance, print the provisioning profile name.
        execute_process(
            OUTPUT_VARIABLE PROFILE_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
            COMMAND security cms -D -i ${BEST_PROFILE}
            COMMAND plutil -extract Name raw -
        )
        message("Using provisioning profile: ${PROFILE_NAME}")

        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND_EXPAND_LISTS
            COMMAND ${COMMENT_ECHO_COMMAND} "Bundling embedded.provisionprofile"
            COMMAND ${CMAKE_COMMAND} -E copy ${BEST_PROFILE} $<TARGET_BUNDLE_CONTENT_DIR:${TARGET}>/embedded.provisionprofile
        )
    endif()
endfunction()
