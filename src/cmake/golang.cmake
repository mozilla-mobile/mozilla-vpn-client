# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Create a library target built from a golang c-archive.
function(add_go_library GOTARGET SOURCE)
    cmake_parse_arguments(GOLANG
        ""
        ""
        "CGO_CFLAGS;CGO_LDFLAGS"
        ${ARGN})
    get_filename_component(SRC_NAME ${SOURCE} NAME)
    get_filename_component(DIR_NAME ${SOURCE} DIRECTORY)
    get_filename_component(DIR_ABSOLUTE ${DIR_NAME} ABSOLUTE)

    file(GLOB_RECURSE SRC_DEPS ${DIR_NAME}/*.go)
    string(REGEX REPLACE "[.]go$" ".h" HEADER_NAME ${SRC_NAME})
    string(REGEX REPLACE "[.]go$" ${CMAKE_STATIC_LIBRARY_SUFFIX} ARCHIVE_NAME ${SRC_NAME})

    set(GOCACHE ${CMAKE_BINARY_DIR}/go-cache)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/go-cache)
    set(GOFLAGS -buildmode=c-archive -v)
    if(IS_DIRECTORY ${DIR_NAME}/vendor)
        set(GOFLAGS ${GOFLAGS} -mod vendor)
    endif()

    ## Add extras to the CGO compiler and linker flags.
    execute_process(OUTPUT_VARIABLE DEFAULT_CGO_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND go env CGO_CFLAGS)
    execute_process(OUTPUT_VARIABLE DEFAULT_CGO_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND go env CGO_LDFLAGS)
    separate_arguments(DEFAULT_CGO_CFLAGS NATIVE_COMMAND ${DEFAULT_CGO_CFLAGS})
    separate_arguments(DEFAULT_CGO_LDFLAGS NATIVE_COMMAND ${DEFAULT_CGO_LDFLAGS})
    list(PREPEND GOLANG_CGO_CFLAGS ${DEFAULT_CGO_CFLAGS})
    list(PREPEND GOLANG_CGO_LDFLAGS ${DEFAULT_CGO_LDFLAGS})

    ## The actual commands that do the building.
    add_custom_target(golang_${GOTARGET}
        BYPRODUCTS ${ARCHIVE_NAME} ${HEADER_NAME}
        WORKING_DIRECTORY ${DIR_ABSOLUTE}
        SOURCES ${SRC_DEPS} ${DIR_NAME}/go.mod
        COMMAND ${CMAKE_COMMAND} -E env GOCACHE=${GOCACHE}
                    CGO_ENABLED=1
                    CC=gcc
                    CGO_CFLAGS="${GOLANG_CGO_CFLAGS}"
                    CGO_LDFLAGS="${GOLANG_CGO_LDFLAGS}"
                go build ${GOFLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/${ARCHIVE_NAME} ${SRC_NAME}
    )

    ## Wrap up the built library as an imported target.
    add_library(${GOTARGET} STATIC IMPORTED GLOBAL)
    add_dependencies(${GOTARGET} golang_${GOTARGET})
    set_target_properties(${GOTARGET} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}
        INTERFACE_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_NAME}
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${ARCHIVE_NAME})

    if(MSVC)
        # prevent error LNK2019: unresolved external symbol fprintf referenced in function ...
        set_property(TARGET ${GOTARGET} APPEND PROPERTY
            INTERFACE_SOURCES ${CMAKE_SOURCE_DIR}/src/platforms/windows/golang-msvc-fixup.cpp)
    endif()

endfunction(add_go_library)
