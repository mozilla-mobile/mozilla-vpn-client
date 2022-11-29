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
    find_program(CODESIGN_BIN NAMES codesign)
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
    if(CODE_SIGN_IDENTITY)
        cmake_parse_arguments(CODESIGN
            "FORCE"
            "ENTITLEMENTS"
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
        if(CODESIGN_ENTITLEMENTS)
            list(APPEND CODESIGN_ARGS --entitlements ${CODESIGN_ENTITLEMENTS})
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
    endif()
endfunction()
