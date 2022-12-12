# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Find the absolute path to the rust build tools.
find_program(CARGO_BUILD_TOOL NAMES cargo REQUIRED)
find_program(RUSTC_BUILD_TOOL NAMES rustc REQUIRED)

# Figure out Rust's host architecture
execute_process(OUTPUT_VARIABLE RUSTC_VERSION_RAW COMMAND ${RUSTC_BUILD_TOOL} --version --verbose)
if(RUSTC_VERSION_RAW MATCHES "host: ([^\n]+)")
    set(RUSTC_HOST_ARCH ${CMAKE_MATCH_1})
else()
    error("Failed to find rustc host arch")
endif()

### Helper function to build Rust static libraries.
#
# Accepts the following arguments:
#   ARCH: Rust target architecture to build with --target ${ARCH}
#   BINARY_DIR: Binary directory to output build artifacts to.
#   PACKAGE_DIR: Soruce directory where Cargo.toml can be found.
#   LIBRARY_FILE: Filename of the expected library to be built.
#   CARGO_ENV_<name>: Environment variables to pass to cargo
#
# This function generates commands necessary to build static archives
# in ${BINARY_DIR}/${ARCH}/debug/ and ${BINARY_DIR}/${ARCH}/release/
# and it is up to the caller of this function to link the artifacts
# into their targets as necessary.
#
function(build_rust_archives)
    cmake_parse_arguments(RUST_BUILD
        ""
        "ARCH;BINARY_DIR;PACKAGE_DIR;LIBRARY_FILE"
        "CARGO_ENV"
        ${ARGN})

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cargo_home)
    list(APPEND RUST_BUILD_CARGO_ENV CARGO_HOME=\"${CMAKE_BINARY_DIR}/cargo_home\")

    # Parse the remaining arguments for environment variables to pass to cargo.
    # This emulates single-valued arguments in the form CARGO_ENV_<name> <value>
    while(RUST_BUILD_UNPARSED_ARGUMENTS)
        list(POP_FRONT RUST_BUILD_UNPARSED_ARGUMENTS NEXT_UNPARSED_ARG)

        if(${NEXT_UNPARSED_ARG} MATCHES "CARGO_ENV_([0-9A-Z_]+)$")
            list(POP_FRONT RUST_BUILD_UNPARSED_ARGUMENTS NEXT_UNPARSED_VALUE)
            list(APPEND RUST_BUILD_CARGO_ENV ${CMAKE_MATCH_1}=\"${NEXT_UNPARSED_VALUE}\")
        else()
            message(WARNING "Unexpected argument: ${NEXT_UNPARSED_ARG}")
        endif()
    endwhile()

    if(NOT RUST_BUILD_LIBRARY_FILE)
        error("Mandatory argument LIBRARY_FILE was not found")
    endif()
    if(NOT RUST_BUILD_ARCH)
        set(RUST_BUILD_ARCH ${RUSTC_HOST_ARCH})
    endif()
    if(NOT RUST_BUILD_BINARY_DIR)
        set(RUST_BUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT RUST_BUILD_PACKAGE_DIR)
        set(RUST_BUILD_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    file(MAKE_DIRECTORY ${RUST_BUILD_BINARY_DIR})

    ## Outputs for the release build
    add_custom_command(
        OUTPUT ${RUST_BUILD_BINARY_DIR}/${RUST_BUILD_ARCH}/release/${RUST_BUILD_LIBRARY_FILE}
        ## DEPFILE ${RUST_BUILD_BINARY_DIR}/${RUST_BUILD_ARCH}/release/
        WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                ${CARGO_BUILD_TOOL} build --lib --release --target ${RUST_BUILD_ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
    )

    ## Outputs for the debug build
    add_custom_command(
        OUTPUT ${RUST_BUILD_BINARY_DIR}/${RUST_BUILD_ARCH}/debug/${RUST_BUILD_LIBRARY_FILE}
        ## DEPFILE ${RUST_BUILD_BINARY_DIR}/${RUST_BUILD_ARCH}/debug/
        WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                ${CARGO_BUILD_TOOL} build --lib --target ${RUST_BUILD_ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
    )
endfunction()
