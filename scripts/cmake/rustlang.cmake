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

## For the Ninja generator, setup a job pool for Cargo targets, which share a
## common lock on the package repository, and build aggressively in parallel
## anyways.
get_property(HAS_CARGO_POOL GLOBAL PROPERTY JOB_POOLS)
list(FILTER HAS_CARGO_POOL INCLUDE REGEX "^cargo=")
if(NOT HAS_CARGO_POOL)
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS cargo=1)
endif()

### Helper function to build Rust static libraries.
#
# Accepts the following arguments:
#   ARCH: Rust target architecture to build with --target ${ARCH}
#   BINARY_DIR: Binary directory to output build artifacts to.
#   PACKAGE_DIR: Soruce directory where Cargo.toml can be found.
#   LIBRARY_FILE: Filename of the expected library to be built.
#   CARGO_ENV: Environment variables to pass to cargo
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
        "ARCH;BINARY_DIR;PACKAGE_DIR;CRATE_NAME"
        "CARGO_ENV"
        ${ARGN})

    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cargo_home)
    list(APPEND RUST_BUILD_CARGO_ENV CARGO_HOME=\"${CMAKE_BINARY_DIR}/cargo_home\")

    if(NOT RUST_BUILD_CRATE_NAME)
        error("Mandatory argument CRATE_NAME was not found")
    endif()
    if(NOT RUST_BUILD_ARCH)
        error("Mandatory argument ARCH was not found")
    endif()
    if(NOT RUST_BUILD_BINARY_DIR)
        set(RUST_BUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT RUST_BUILD_PACKAGE_DIR)
        set(RUST_BUILD_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    ## Some files that we will be building.
    file(MAKE_DIRECTORY ${RUST_BUILD_BINARY_DIR})
    set(RUST_BUILD_LIBRARY_FILE
        ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_BUILD_CRATE_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
    )

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
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
            set(ANDROID_HOST_TAG linux-x86_64)
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
            set(ANDROID_HOST_TAG darwin-x86_64)
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
            set(ANDROID_HOST_TAG windows-x86_64)
        else()
            message("Unsupported OS?")
        endif()  
        # Toolchain.
        set(ANDROID_TOOLCHAIN_ROOT
            "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${ANDROID_HOST_TAG}")
       
        # Rust and Clang disagree about armv7, otherwise we can take the Rust build arch. 
        if(${RUST_BUILD_ARCH} STREQUAL armv7-linux-androideabi)
            set(ANDROID_ARCH_NAME armv7a-linux-androideabi)
        else()
            set(ANDROID_ARCH_NAME ${RUST_BUILD_ARCH})
        endif()

        
        list(APPEND RUST_BUILD_CARGO_ENV CC=${ANDROID_TOOLCHAIN_ROOT}/bin/${ANDROID_ARCH_NAME}${ANDROID_NATIVE_API_LEVEL}-clang)
    endif()

    if(CMAKE_GENERATOR MATCHES "Ninja")
        ## If we are building with Ninja, then we can improve build times by
        # specifying a DEPFILE to let CMake know when the library needs
        # building and when we can skip it.
        #
        # TODO: Since CMake 3.20 can also set a DEPFILE for Unix Makefiles too.
        set(RUST_BUILD_DEPENDENCY_FILE
            ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_BUILD_CRATE_NAME}.d
        )
        cmake_policy(PUSH)
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.20)
            cmake_policy(SET CMP0116 NEW)
        endif()

        ## Outputs for the release build
        add_custom_command(
            OUTPUT ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/${RUST_BUILD_LIBRARY_FILE}
            DEPFILE ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/${RUST_BUILD_DEPENDENCY_FILE}
            JOB_POOL cargo
            WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                    ${CARGO_BUILD_TOOL} build --lib --release --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
        )

        ## Outputs for the debug build
        add_custom_command(
            OUTPUT ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/${RUST_BUILD_LIBRARY_FILE}
            DEPFILE ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/${RUST_BUILD_DEPENDENCY_FILE}
            JOB_POOL cargo
            WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                    ${CARGO_BUILD_TOOL} build --lib --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
        )

        ## Reset our policy changes
        cmake_policy(POP)
    else()
        ## For all other generators, set a non-existent output file to force
        # the command to be invoked on every build. This ensures that the
        # library stays up todate with the sources, and relies on cargo to
        # rebuild if necessary.

        ## Outputs for the release build
        add_custom_command(
            OUTPUT
                ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/${RUST_BUILD_LIBRARY_FILE}
                ${RUST_BUILD_BINARY_DIR}/${ARCH}/release/.noexist
            WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                    ${CARGO_BUILD_TOOL} build --lib --release --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
        )

        ## Outputs for the debug build
        add_custom_command(
            OUTPUT
                ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/${RUST_BUILD_LIBRARY_FILE}
                ${RUST_BUILD_BINARY_DIR}/${ARCH}/debug/.noexist
            WORKING_DIRECTORY ${RUST_BUILD_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E env ${RUST_BUILD_CARGO_ENV}
                    ${CARGO_BUILD_TOOL} build --lib --target ${ARCH} --target-dir ${RUST_BUILD_BINARY_DIR}
        )
    endif()
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
#   CRATE_NAME: Name of the staticlib crate we want to build.
#   CARGO_ENV: Environment variables to pass to cargo
#   DEPENDS: Additional files on which the target depends.
#
function(add_rust_library TARGET_NAME)
    cmake_parse_arguments(RUST_TARGET
        ""
        "BINARY_DIR;PACKAGE_DIR;CRATE_NAME"
        "ARCH;CARGO_ENV;DEPENDS"
        ${ARGN})

    add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)

    if(NOT RUST_TARGET_CRATE_NAME)
        ## TODO: I would like to pull this from the package manifest.
        error("Mandatory argument CRATE_NAME was not found")
    endif()
    if(NOT RUST_TARGET_BINARY_DIR)
        set(RUST_TARGET_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT RUST_TARGET_PACKAGE_DIR)
        set(RUST_TARGET_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    # Guess the target architecture if not set.
    if(NOT RUST_TARGET_ARCH)
        if(CMAKE_CROSSCOMPILING)
            # TODO: We could write something here for Android and IOS maybe
            message(FATAL_ERROR "Unable to determine rust target architecture when cross compiling.")
        elseif((CMAKE_SYSTEM_NAME STREQUAL "Darwin") AND CMAKE_OSX_ARCHITECTURES)
            # Special case for MacOS universal binaries.
            foreach(OSXARCH ${CMAKE_OSX_ARCHITECTURES})
                string(REPLACE "arm64" "aarch64" OSXARCH ${OSXARCH})
                list(APPEND RUST_TARGET_ARCH "${OSXARCH}-apple-darwin")
            endforeach()
        else()
            set(RUST_TARGET_ARCH ${RUSTC_HOST_ARCH})
        endif()
    endif()

    set(RUST_TARGET_LIBRARY_FILE
        ${CMAKE_STATIC_LIBRARY_PREFIX}${RUST_TARGET_CRATE_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
    )

    ## Build the rust library file(s)
    foreach(ARCH ${RUST_TARGET_ARCH})
        build_rust_archives(
            ARCH ${ARCH}
            BINARY_DIR ${RUST_TARGET_BINARY_DIR}
            PACKAGE_DIR ${RUST_TARGET_PACKAGE_DIR}
            CRATE_NAME ${RUST_TARGET_CRATE_NAME}
            CARGO_ENV ${CARGO_ENV}
        )

        if(RUST_TARGET_DEPENDS)
            add_custom_command(APPEND
                OUTPUT ${RUST_TARGET_BINARY_DIR}/${ARCH}/release/${RUST_TARGET_LIBRARY_FILE}
                DEPENDS ${RUST_TARGET_DEPENDS}
            )
            add_custom_command(APPEND
                OUTPUT ${RUST_TARGET_BINARY_DIR}/${ARCH}/debug/${RUST_TARGET_LIBRARY_FILE}
                DEPENDS ${RUST_TARGET_DEPENDS}
            )
        endif()

        # Keep track of the expected library artifacts.
        list(APPEND RUST_TARGET_RELEASE_LIBS ${RUST_TARGET_BINARY_DIR}/${ARCH}/release/${RUST_TARGET_LIBRARY_FILE})
        list(APPEND RUST_TARGET_DEBUG_LIBS ${RUST_TARGET_BINARY_DIR}/${ARCH}/debug/${RUST_TARGET_LIBRARY_FILE})
    endforeach()
    
    if(APPLE)
        ## For apple platforms unify the built libraries
        add_custom_command(
            OUTPUT ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_LIBRARY_FILE}
            DEPENDS ${RUST_TARGET_RELEASE_LIBS}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RUST_TARGET_BINARY_DIR}/unified/release
            COMMAND lipo -create -output ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_LIBRARY_FILE}
                        ${RUST_TARGET_RELEASE_LIBS}
        )
        add_custom_command(
            OUTPUT ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_LIBRARY_FILE}
            DEPENDS ${RUST_TARGET_DEBUG_LIBS}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${RUST_TARGET_BINARY_DIR}/unified/debug
            COMMAND lipo -create -output ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_LIBRARY_FILE}
                        ${RUST_TARGET_DEBUG_LIBS}
        )

        add_custom_target(${TARGET_NAME}_builder
            DEPENDS ${RUST_TARGET_BINARY_DIR}/unified/$<IF:$<CONFIG:Debug>,debug,release>/${RUST_TARGET_LIBRARY_FILE}
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION ${RUST_TARGET_BINARY_DIR}/unified/release/${RUST_TARGET_LIBRARY_FILE}
            IMPORTED_LOCATION_DEBUG ${RUST_TARGET_BINARY_DIR}/unified/debug/${RUST_TARGET_LIBRARY_FILE}
        )
    else()
        ## For all other platforms, only build the first architecture
        list(GET RUST_TARGET_ARCH 0 RUST_FIRST_ARCH)
        add_custom_target(${TARGET_NAME}_builder
            DEPENDS ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/$<IF:$<CONFIG:Debug>,debug,release>/${RUST_TARGET_LIBRARY_FILE}
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/release/${RUST_TARGET_LIBRARY_FILE}
            IMPORTED_LOCATION_DEBUG ${RUST_TARGET_BINARY_DIR}/${RUST_FIRST_ARCH}/debug/${RUST_TARGET_LIBRARY_FILE}
        )
    endif()
    set_target_properties(${TARGET_NAME}_builder PROPERTIES FOLDER "Libs")

    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_builder)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
endfunction()
