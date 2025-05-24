# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## The contents of this file should only be processed once.
include_guard(GLOBAL)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cargo_home)

## Find the absolute path to the rust build tools.
find_program(CARGO_BUILD_TOOL NAMES cargo REQUIRED)

## Don't trust Xcode to provide us with a usable linker.
if(APPLE AND XCODE)
    find_program(RUSTC_BUILD_TOOL_PATH NAMES rustc REQUIRED)
    file(WRITE ${CMAKE_BINARY_DIR}/cargo_home/rustwrapper.sh "#!/bin/sh\n")
    file(APPEND ${CMAKE_BINARY_DIR}/cargo_home/rustwrapper.sh "${RUSTC_BUILD_TOOL_PATH} -C linker=/usr/bin/cc \$@\n")
    file(CHMOD ${CMAKE_BINARY_DIR}/cargo_home/rustwrapper.sh FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    set(RUSTC_BUILD_TOOL ${CMAKE_BINARY_DIR}/cargo_home/rustwrapper.sh
        CACHE FILEPATH "Path to the rustc build tool"
    )
else()
    find_program(RUSTC_BUILD_TOOL NAMES rustc REQUIRED)
endif()

# Figure out Rust's host architecture
execute_process(OUTPUT_VARIABLE RUSTC_VERSION_RAW COMMAND ${RUSTC_BUILD_TOOL} --version --verbose)
if(RUSTC_VERSION_RAW MATCHES "host: ([^\n]+)")
    set(RUSTC_HOST_ARCH ${CMAKE_MATCH_1})
else()
    message(FATAL_ERROR "Failed to find rustc host arch")
endif()

## For the Ninja generator, setup a job pool for Cargo targets, which share a
## common lock on the package repository, and build aggressively in parallel
## anyways.
get_property(HAS_CARGO_POOL GLOBAL PROPERTY JOB_POOLS)
list(FILTER HAS_CARGO_POOL INCLUDE REGEX "^cargo=")
if(NOT HAS_CARGO_POOL)
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS cargo=1)
endif()

## We want to make use of DEPFILE support when building rust crates as it speeds
## up the build jobs fairly significantly. This is supported by most generators
## so let's just make extra sure that we are using one.
if((CMAKE_GENERATOR MATCHES "Ninja") OR (CMAKE_GENERATOR MATCHES "Makefiles"))
    # Ninja generators support DEPFILEs since CMake 3.7 (3.17 for multi-config).
    # Makefile generators support DEPFILEs as of CMake 3.20.
    set(HAS_DEPFILE_SUPPORT TRUE)
elseif(XCODE OR CMAKE_VS_VERSION_BUILD_NUMBER)
    # Xcode and Visual Studio 2012 and later support DEPFILEs as of CMake 3.21.
    set(HAS_DEPFILE_SUPPORT TRUE)
else()
    message(FATAL_ERROR "CMake generator is missing DEPFILE support")
endif()

## Create a target to build the cbindgen tool.
## TODO: This should be versioned by the Cargo.lock, if present.
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/cargo_home/bin/cbindgen
    COMMENT "Building rust cbindgen tool"
    JOB_POOL cargo
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E env CARGO_HOME=${CMAKE_BINARY_DIR}/cargo_home RUSTC=${RUSTC_BUILD_TOOL}
            ${CARGO_BUILD_TOOL} install --root ${CMAKE_BINARY_DIR}/cargo_home cbindgen
)
set(CBINDGEN_BUILD_TOOL ${CMAKE_BINARY_DIR}/cargo_home/bin/cbindgen
    CACHE FILEPATH "Path to the cbindgen build tool"
)

### Helper function to parse a Rust manifest
#
# Accepts the following arguments:
#   PACKAGE_DIR: Source directory where Cargo.toml can be found.
#   CRATE_TARGET: Which target to parse from the manifest, defaults to the package name.
#
# Outputs the following values:
#   OUTPUT_LIB_NAME: Output variable name to receive the library name. 
#   OUTPUT_LIB_KINDS: Output variable name to receive a list of target kinds.
#
function(parse_rust_manifest)
    cmake_parse_arguments(RUST_PARSE
        ""
        "PACKAGE_DIR;OUTPUT_LIB_NAME;OUTPUT_LIB_KINDS"
        ""
        ${ARGN})

    if(NOT RUST_PARSE_PACKAGE_DIR)
        set(RUST_PARSE_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Parse the crate manifest to figure out what we're building.
    execute_process(
        WORKING_DIRECTORY ${RUST_PARSE_PACKAGE_DIR}
        OUTPUT_VARIABLE RUST_PARSE_CRATE_JSON
        COMMAND ${CARGO_BUILD_TOOL} read-manifest 
    )
    string(JSON RUST_PARSE_TARGET_JSON GET ${RUST_PARSE_CRATE_JSON} "targets")
    string(JSON RUST_PARSE_TARGET_COUNT LENGTH ${RUST_PARSE_TARGET_JSON})

    # Look for the targets entry in the parsed manifest.
    set(RUST_PARSE_TARGET_INDEX 0)
    while(${RUST_PARSE_TARGET_INDEX} LESS ${RUST_PARSE_TARGET_COUNT})
        string(JSON JSON_TARGET GET ${RUST_PARSE_TARGET_JSON} ${RUST_PARSE_TARGET_INDEX})
        string(JSON TARGET_NAME GET ${JSON_TARGET} "name")
        math(EXPR RUST_PARSE_TARGET_INDEX "${RUST_PARSE_TARGET_INDEX} + 1")

        # Parse the target kinds
        set(KIND_INDEX 0)
        set(KIND_LIST)
        string(JSON KIND_COUNT LENGTH ${JSON_TARGET} "kind")
        while(${KIND_INDEX} LESS ${KIND_COUNT})
            string(JSON KIND_TYPE GET ${JSON_TARGET} "kind" ${KIND_INDEX})
            math(EXPR KIND_INDEX "${KIND_INDEX} + 1")
            list(APPEND KIND_LIST ${KIND_TYPE})
        endwhile()

        # Check if this is the library target - crates can have at most one.
        if(("staticlib" IN_LIST KIND_LIST) OR ("cdylib" IN_LIST KIND_LIST))
            if(RUST_PARSE_OUTPUT_LIB_NAME)
                set(${RUST_PARSE_OUTPUT_LIB_NAME} ${TARGET_NAME} PARENT_SCOPE)
            endif()
            if(RUST_PARSE_OUTPUT_LIB_KINDS)
                set(${RUST_PARSE_OUTPUT_LIB_KINDS} ${KIND_LIST} PARENT_SCOPE)
            endif()
        endif()
    endwhile()
endfunction()

### Helper function to build a Rust library target
#
# Accepts the following arguments:
#   ARCH: Rust target architecture to build with --target ${ARCH}
#   BINARY_DIR: Binary directory to output build artifacts to.
#   PACKAGE_DIR: Source directory where Cargo.toml can be found.
#   CARGO_ENV: Environment variables to pass to cargo
#   DEPENDS: Additional dependencies required to build the library.
#
# This function generates commands necessary to build static archives
# in ${BINARY_DIR}/${ARCH}/debug/ and ${BINARY_DIR}/${ARCH}/release/
# and it is up to the caller of this function to link the artifacts
# into their targets as necessary.
#
# This function is intended to be used internally by add_rust_library,
# you should consider using that instead.
#
function(build_rust_archives)
    cmake_parse_arguments(RUST_BUILD
        ""
        "ARCH;BINARY_DIR;PACKAGE_DIR"
        "CARGO_ENV;DEPENDS"
        ${ARGN})

    list(APPEND RUST_BUILD_CARGO_ENV CARGO_HOME=\"${CMAKE_BINARY_DIR}/cargo_home\")
    list(APPEND RUST_BUILD_CARGO_ENV RUSTC=${RUSTC_BUILD_TOOL})

    if(NOT RUST_BUILD_ARCH)
        message(FATAL_ERROR "Mandatory argument ARCH was not found")
    endif()
    if(NOT RUST_BUILD_BINARY_DIR)
        set(RUST_BUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT RUST_BUILD_PACKAGE_DIR)
        set(RUST_BUILD_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Parse the crate manifest for the kinds of targets we are building.
    set(RUST_BUILD_DEBUG_OUTPUT_FILES)
    set(RUST_BUILD_RELEASE_OUTPUT_FILES)
    parse_rust_manifest(
        PACKAGE_DIR ${RUST_BUILD_PACKAGE_DIR}
        OUTPUT_LIB_NAME RUST_BUILD_LIB_NAME
        OUTPUT_LIB_KINDS RUST_BUILD_LIB_KINDS
    )
    foreach(KIND ${RUST_BUILD_LIB_KINDS})
        set(TARGET_FILENAME)
        if(KIND STREQUAL "staticlib")
            set(TARGET_FILENAME ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_BUILD_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX})
        elseif(KIND STREQUAL "cdylib")
            set(TARGET_FILENAME ${CMAKE_SHARED_LIBRARY_PREFIX}${RUST_BUILD_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
        else()
            continue()
        endif()
        list(APPEND RUST_BUILD_DEBUG_OUTPUT_FILES ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/${TARGET_FILENAME})
        list(APPEND RUST_BUILD_RELEASE_OUTPUT_FILES ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/${TARGET_FILENAME})
    endforeach()

    ## Some files that we will be building.
    file(MAKE_DIRECTORY ${RUST_BUILD_BINARY_DIR})

    ## For iOS simulator targets, ensure that we unset the `SDKROOT` variable as
    ## this will result in broken simulation builds in Xcode. For all other apple
    ## platforms, attempt to set the SDKROOT via CMAKE_OSX_SYSROOT
    if((RUST_BUILD_ARCH STREQUAL "aarch64-apple-ios-sim") OR (RUST_BUILD_ARCH STREQUAL "x86_64-apple-ios"))
        list(PREPEND RUST_BUILD_CARGO_ENV "--unset=SDKROOT")
    elseif(APPLE AND CMAKE_OSX_SYSROOT)
        execute_process(OUTPUT_VARIABLE RUST_BUILD_SDKROOT OUTPUT_STRIP_TRAILING_WHITESPACE
            COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path)
        list(APPEND RUST_BUILD_CARGO_ENV "SDKROOT=${RUST_BUILD_SDKROOT}")
    endif()

    ## For MacOS platforms, set the OS deployment version.
    if(RUST_BUILD_ARCH MATCHES "-apple-darwin$")
        list(APPEND RUST_BUILD_CARGO_ENV MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    endif()

    if(ANDROID)
        get_filename_component(ANDROID_TOOLCHAIN_ROOT_BIN ${CMAKE_C_COMPILER} DIRECTORY)

        # Rust and Clang disagree about armv7, otherwise we can take the Rust build arch.
        if(${RUST_BUILD_ARCH} STREQUAL armv7-linux-androideabi)
            set(ANDROID_ARCH_NAME armv7a-linux-androideabi)
        else()
            set(ANDROID_ARCH_NAME ${RUST_BUILD_ARCH})
        endif()

        list(APPEND RUST_BUILD_CARGO_ENV RUSTFLAGS=-Clinker=${ANDROID_TOOLCHAIN_ROOT_BIN}/${ANDROID_ARCH_NAME}${ANDROID_NATIVE_API_LEVEL}-clang)
        list(APPEND RUST_BUILD_CARGO_ENV CC=${ANDROID_TOOLCHAIN_ROOT_BIN}/${ANDROID_ARCH_NAME}${ANDROID_NATIVE_API_LEVEL}-clang)
        list(APPEND RUST_BUILD_CARGO_ENV AR=${ANDROID_TOOLCHAIN_ROOT_BIN}/llvm-ar)
        list(APPEND RUST_BUILD_CARGO_ENV LD=${ANDROID_TOOLCHAIN_ROOT_BIN}/lld)
    endif()

    cmake_policy(PUSH)
    cmake_policy(SET CMP0116 NEW)
    set(RUST_BUILD_DEPENDENCY_FILE ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_BUILD_LIB_NAME}.d)

    ## Outputs for the release build
    add_custom_command(
        OUTPUT ${RUST_BUILD_RELEASE_OUTPUT_FILES}
        DEPENDS ${RUST_BUILD_DEPENDS}
        DEPFILE ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/${RUST_BUILD_DEPENDENCY_FILE}
        JOB_POOL cargo
        WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                ${CARGO_BUILD_TOOL} build --lib --release --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
    )

    ## Outputs for the debug build
    add_custom_command(
        OUTPUT ${RUST_BUILD_DEBUG_OUTPUT_FILES}
        DEPENDS ${RUST_BUILD_DEPENDS}
        DEPFILE ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/${RUST_BUILD_DEPENDENCY_FILE}
        JOB_POOL cargo
        WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                ${CARGO_BUILD_TOOL} build --lib --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
    )

    ## Reset our policy changes
    cmake_policy(POP)
endfunction()

### Helper function to create a linkable target from a Rust package.
#
# This function takes one mandatory argument: TARGET_NAME which
# sets the name of the CMake target to produce.
#
# Accepts the following optional arguments:
#   ARCH: Rust target architecture(s) to build with --target ${ARCH}
#   BINARY_DIR: Binary directory to output build artifacts to.
#   PACKAGE_DIR: Soruce directory where Cargo.toml can be found.
#   CARGO_ENV: Environment variables to pass to cargo.
#   DEPENDS: Additional files on which the target depends.
#   SHARED: Whether or not we are building a shared library. Defaults to "false".
#   FW_NAME: Standalone dylibs need to be wrapped in a framework for distribtuion. Required when building shared lib for iOS.
#
function(add_rust_library TARGET_NAME)
    cmake_parse_arguments(RUST_TARGET
        ""
        "BINARY_DIR;PACKAGE_DIR"
        "ARCH;CARGO_ENV;DEPENDS;SHARED;FW_NAME"
        ${ARGN})

    if(NOT RUST_TARGET_SHARED)
        set(RUST_TARGET_SHARED 0)
    endif()

    if(${RUST_TARGET_SHARED} AND IOS AND NOT RUST_TARGET_FW_NAME)
        message(FATAL_ERROR "A framework name must be provided when building a shared Rust library for iOS.")
    endif()

    if(IOS AND RUST_TARGET_FW_NAME)
        set(FW_INFO_PLIST_FILE_PATH ${CMAKE_SOURCE_DIR}/scripts/cmake/Info.plist.${RUST_TARGET_FW_NAME})

        if(NOT EXISTS ${FW_INFO_PLIST_FILE_PATH})
            message(FATAL_ERROR "An Info.plist.${RUST_TARGET_FW_NAME} file must exist to support creation of ${FW_NAME} framework.")
        endif()

        list(APPEND RUST_TARGET_CARGO_ENV "RUSTC_LINK_ARG=-Wl,-install_name,@rpath/${RUST_TARGET_FW_NAME}.framework/${RUST_TARGET_FW_NAME}")
    endif()

    if(${RUST_TARGET_SHARED})
        add_library(${TARGET_NAME} SHARED IMPORTED GLOBAL)
    else()
        add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
    endif()

    if(NOT RUST_TARGET_BINARY_DIR)
        set(RUST_TARGET_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT RUST_TARGET_PACKAGE_DIR)
        set(RUST_TARGET_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    ## Parse the crate manifest to determine the library being built.
    parse_rust_manifest(
        PACKAGE_DIR ${RUST_TARGET_PACKAGE_DIR}
        OUTPUT_LIB_NAME RUST_TARGET_LIB_NAME
    )
    if(NOT RUST_TARGET_LIB_NAME)
        message(FATAL_ERROR "No library targets found in ${RUST_TARGET_PACKAGE_DIR}")
    endif()
    if(RUST_TARGET_SHARED)
        set(RUST_TARGET_FILENAME
            ${CMAKE_SHARED_LIBRARY_PREFIX}${RUST_TARGET_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        )
    else()
        set(RUST_TARGET_FILENAME
            ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_TARGET_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
        )
    endif()

    ## Generate a library header with cbindgen
    file(MAKE_DIRECTORY ${RUST_TARGET_BINARY_DIR}/include/bindings)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${RUST_TARGET_BINARY_DIR}/include
    )
    add_custom_command(
        OUTPUT ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
        BYPRODUCTS ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_LIB_NAME}-bindings.d
        DEPENDS ${CBINDGEN_BUILD_TOOL}
        DEPFILE ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_LIB_NAME}-bindings.d
        WORKING_DIRECTORY ${RUST_TARGET_PACKAGE_DIR}
        COMMAND ${CBINDGEN_BUILD_TOOL}
                    -o ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
                    --depfile ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_LIB_NAME}-bindings.d
    )
    set_source_files_properties(
        ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
        PROPERTIES GENERATED TRUE
    )

    # Guess the target architecture if not set.
    if(NOT RUST_TARGET_ARCH)
        if(ANDROID)
            if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
                set(RUST_TARGET_ARCH "armv7-linux-androideabi")
            else()
                set(RUST_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}-linux-android")
            endif()
        elseif(IOS)
            ## TODO: This doesn't work for aarch64 simulators.
            # See also: https://bugreports.qt.io/browse/QTBUG-101276
            foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
                string(REPLACE "arm64" "aarch64" OSXARCH ${OSXARCH})
                list(APPEND RUST_TARGET_ARCH "${OSXARCH}-apple-ios")
            endforeach()
            # Build all architectures if nothing is set.
            if(NOT RUST_TARGET_ARCH)
                set(RUST_TARGET_ARCH aarch64-apple-ios x86_64-apple-ios)
            endif()
        elseif((CMAKE_SYSTEM_NAME STREQUAL "Darwin") AND CMAKE_OSX_ARCHITECTURES)
            # Special case for MacOS universal binaries.
            foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
                string(REPLACE "arm64" "aarch64" OSXARCH ${OSXARCH})
                list(APPEND RUST_TARGET_ARCH "${OSXARCH}-apple-darwin")
            endforeach()
        elseif(NOT CMAKE_CROSSCOMPILING)
            set(RUST_TARGET_ARCH ${RUSTC_HOST_ARCH})
        else()
            message(FATAL_ERROR "Unable to determine rust target architecture.")
        endif()
    endif()

    ## Build the rust library target(s)
    foreach(ARCH ${RUST_TARGET_ARCH})
        build_rust_archives(
            ARCH ${ARCH}
            BINARY_DIR ${RUST_TARGET_BINARY_DIR}
            PACKAGE_DIR ${RUST_TARGET_PACKAGE_DIR}
            CARGO_ENV ${RUST_TARGET_CARGO_ENV}
            DEPENDS ${RUST_TARGET_DEPENDS}
        )

        # Keep track of the expected library artifacts.
        list(APPEND RUST_TARGET_RELEASE_LIBS ${RUST_TARGET_BINARY_DIR}/${ARCH}/release/${RUST_TARGET_FILENAME})
        list(APPEND RUST_TARGET_DEBUG_LIBS ${RUST_TARGET_BINARY_DIR}/${ARCH}/debug/${RUST_TARGET_FILENAME})
    endforeach()

    if(IOS AND RUST_TARGET_FW_NAME)
        # For iOS frameworks merge the release libraries together with lipo
        # and bundle it together into a .framework directory.
        add_custom_command(
            OUTPUT ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_FW_NAME}.framework/${RUST_TARGET_FW_NAME}
            WORKING_DIRECTORY ${RUST_TARGET_BINARY_DIR}
            DEPENDS ${RUST_TARGET_RELEASE_LIBS}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RUST_TARGET_FW_NAME}.framework
            COMMAND ${CMAKE_COMMAND} -E copy ${FW_INFO_PLIST_FILE_PATH} ${RUST_TARGET_FW_NAME}.framework/Info.plist
            COMMAND lipo -create ${RUST_TARGET_RELEASE_LIBS}
                        -output ${RUST_TARGET_FW_NAME}.framework/${RUST_TARGET_FW_NAME}
        )

        add_custom_target(${TARGET_NAME}_builder DEPENDS
            ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
            ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_FW_NAME}.framework/${RUST_TARGET_FW_NAME}
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION ${RUST_TARGET_BINARY_DIR}/${RUST_TARGET_FW_NAME}.framework/${RUST_TARGET_FW_NAME}
        )
    elseif(APPLE)
        # For all other Apple targets, merge the compiled librares together with
        # lipo into a universal library.
        add_custom_command(
            OUTPUT ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_FILENAME}
            DEPENDS ${RUST_TARGET_RELEASE_LIBS}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RUST_TARGET_BINARY_DIR}/unified/release
            COMMAND lipo -create -output ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_FILENAME}
                        ${RUST_TARGET_RELEASE_LIBS}
        )
        add_custom_command(
            OUTPUT ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_FILENAME}
            DEPENDS ${RUST_TARGET_DEBUG_LIBS}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RUST_TARGET_BINARY_DIR}/unified/debug
            COMMAND lipo -create -output ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_FILENAME}
                        ${RUST_TARGET_DEBUG_LIBS}
        )

        add_custom_target(${TARGET_NAME}_builder DEPENDS
            ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
            ${RUST_TARGET_BINARY_DIR}/unified/$<IF:$<CONFIG:Debug>,debug,release>/${RUST_TARGET_FILENAME}
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_FILENAME}
            IMPORTED_LOCATION_DEBUG ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_FILENAME}
        )
    else()
        ## For all other platforms, only build the first architecture
        list(GET RUST_TARGET_ARCH 0 RUST_FIRST_ARCH)
        add_custom_target(${TARGET_NAME}_builder DEPENDS
            ${RUST_TARGET_BINARY_DIR}/include/bindings/${RUST_TARGET_LIB_NAME}.h
            ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/$<IF:$<CONFIG:Debug>,debug,release>/${RUST_TARGET_FILENAME}
        )

        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/release/${RUST_TARGET_FILENAME}
            IMPORTED_LOCATION_DEBUG ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/debug/${RUST_TARGET_FILENAME}
        )
    endif()
    set_target_properties(${TARGET_NAME}_builder PROPERTIES FOLDER "Libs")
    if (ANDROID AND RUST_TARGET_SHARED)
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_NO_SONAME TRUE
        )
    endif()

    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_builder)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
endfunction()
