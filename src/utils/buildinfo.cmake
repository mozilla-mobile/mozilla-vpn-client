# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/. */

string(TIMESTAMP BUILD_TIMESTAMP %s)

# Find the cmake cache and load values from it
while(IS_DIRECTORY ${CMAKE_BINARY_DIR})
    if(EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
        load_cache(${CMAKE_BINARY_DIR} READ_WITH_PREFIX ""
            CMAKE_PROJECT_VERSION
            BUILD_ID
        )
        break()
    endif()
    
    cmake_path(GET CMAKE_BINARY_DIR PARENT_PATH NEXT_BINARY_DIR)
    if (CMAKE_BINARY_DIR STREQUAL NEXT_BINARY_DIR)
        message(FATAL_ERROR "Failed to find CMakeCache.txt")
    endif()
    set(CMAKE_BINARY_DIR ${NEXT_BINARY_DIR})
endwhile()

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE GIT_ROOT_EXISTS
    OUTPUT_QUIET
    ERROR_QUIET
    COMMAND git rev-parse --show-toplevel
)
if(GIT_ROOT_EXISTS EQUAL 0)
    # Gather the git commit information.
    execute_process(
        OUTPUT_VARIABLE BUILD_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND git -C ${CMAKE_CURRENT_LIST_DIR} rev-parse HEAD
    )
    execute_process(
        OUTPUT_VARIABLE GIT_TIMESTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND git -C ${CMAKE_CURRENT_LIST_DIR} log --format=%ct -1 HEAD
    )
    if(GIT_TIMESTAMP GREATER BUILD_TIMESTAMP)
        set(BUILD_TIMESTAMP GIT_TIMESTAMP)
    endif()
endif()

# Generate the file
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/buildinfo.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.cpp
    @ONLY
)
