# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(add_go_library TARGET SOURCE)
    get_filename_component(SRC_NAME ${SOURCE} NAME)
    get_filename_component(DIR_NAME ${SOURCE} DIRECTORY)

    file(GLOB_RECURSE SRC_DEPS ${DIR_NAME}/*.go)
    string(REGEX REPLACE "[.]go$" ".h" HEADER_NAME ${SRC_NAME})
    if(WIN32)
        string(REGEX REPLACE "[.]go$" ".lib" ARCHIVE_NAME ${SRC_NAME})
    else()
        string(REGEX REPLACE "[.]go$" ".a" ARCHIVE_NAME ${SRC_NAME})
    endif()

    target_sources(${TARGET} PRIVATE ${HEADER_NAME})
    set_source_files_properties(${HEADER_NAME} PROPERTIES GENERATED 1)

    if(IS_DIRECTORY $ENV{HOME})
        execute_process(COMMAND go env GOCACHE OUTPUT_VARIABLE GOCACHE OUTPUT_STRIP_TRAILING_WHITESPACE)
    else()
        set(GOCACHE ${CMAKE_BINARY_DIR}/go-cache)
        set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/go-cache)
    endif()
    set(GOFLAGS -buildmode=c-archive -v)
    if(IS_DIRECTORY ${DIR_NAME}/vendor)
        set(GOFLAGS ${GOFLAGS} -mod vendor)
    endif()

    if(MSVC AND NOT (MSVC_VERSION LESS 1900))
        # prevent error LNK2019: unresolved external symbol fprintf referenced in function ...
        target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/windows/golang-msvc-fixup.cpp)
    endif()

    get_filename_component(DIR_ABSOLUTE ${DIR_NAME} ABSOLUTE)
    add_custom_command(
        OUTPUT ${ARCHIVE_NAME} ${HEADER_NAME}
        WORKING_DIRECTORY ${DIR_ABSOLUTE}
        MAIN_DEPENDENCY ${SOURCE}
        DEPENDS ${SRC_DEPS} ${DIR_NAME}/go.mod
        COMMAND ${CMAKE_COMMAND} -E env GOCACHE=${GOCACHE}
                go build ${GOFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/${ARCHIVE_NAME} ${SRC_NAME}
    )
    target_link_libraries(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${ARCHIVE_NAME})
endfunction(add_go_library)
