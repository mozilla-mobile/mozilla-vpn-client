# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Find the absolute path cargo.
## Mostly necessary for iOS builds, 
## because Xcode will attempt to use it's own installation of cargo, 
## which we don't want.
find_program(CARGO_EXECUTABLE NAMES cargo REQUIRED)

add_library(vpnglean STATIC IMPORTED GLOBAL)

set_target_properties(vpnglean PROPERTIES FOLDER "Libs")

# Figure out Rust's host architecture
execute_process(OUTPUT_VARIABLE RUSTC_VERSION_RAW COMMAND rustc --version --verbose)
if(RUSTC_VERSION_RAW MATCHES "host: ([^\n]+)")
    set(RUSTC_HOST_ARCH ${CMAKE_MATCH_1})
else()
    error("Failed to find rustc host arch")
endif()

if(NOT CMAKE_CROSSCOMPILING)
    set(RUST_ARCH ${RUSTC_HOST_ARCH})
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        set(RUST_ARCH "aarch64-linux-android")
        set(OPENSSL_ARCH "arm64")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
        set(RUST_ARCH "armv7-linux-androideabi")
        set(OPENSSL_ARCH "arm")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
        set(RUST_ARCH "i686-linux-android")
        set(OPENSSL_ARCH "x86")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
        set(RUST_ARCH "x86_64-linux-android")
        set(OPENSSL_ARCH "x86_64")
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    if(${CMAKE_OSX_SYSROOT} STREQUAL "iphonesimulator")
        set(RUST_ARCH "x86_64-apple-ios")
    else()
        set(RUST_ARCH "aarch64-apple-ios")
    endif()
endif()

if(NOT RUST_ARCH)
    message(FATAL_ERROR "Unable to determine Rust target for ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR}).")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(OPENSSL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/openSSL/static/lib/${OPENSSL_ARCH})
    set(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/openSSL/static/include)
    set(CARGO_CMD OPENSSL_LIB_DIR=${OPENSSL_LIB_DIR} OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR} AR=llvm-ar ${CARGO_EXECUTABLE} build)
else()
    set(CARGO_CMD ${CARGO_EXECUTABLE} build)
endif()

set(HEADER_FILE vpnglean.h)
set(LIBNAME ${CMAKE_STATIC_LIBRARY_PREFIX}vpnglean${CMAKE_STATIC_LIBRARY_SUFFIX})

## Build the vpnglean Rust library
add_custom_target(vpnglean_ffi
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
    COMMAND ${CMAKE_COMMAND} -E env
                BUILD_ID=${BUILD_ID}
                APP_VERSION=${CMAKE_PROJECT_VERSION}
                CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
                PATH="$<SHELL_PATH:$ENV{PATH}>"
            ${CARGO_CMD} $<IF:$<CONFIG:Debug>,,--release> --target ${RUST_ARCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(vpnglean_ffi PROPERTIES FOLDER "Libs")

# This options should be used if the glean_parser command needs to be run on some sort of pre-build script.
if(NOT SKIP_GLEAN_PARSER)
    set(GENERATE_GLEAN_CMD python3 ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py)
else()
    set(GENERATE_GLEAN_CMD echo "Skipping glean_parser step.")
endif()

## Generate metrics and pings files
add_custom_target(vpnglean_telemetry
    BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/metrics.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/pings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/metrics.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/pings.rs
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py
        ${CMAKE_SOURCE_DIR}/glean/pings.yaml
        ${CMAKE_SOURCE_DIR}/glean/metrics.yaml
    COMMAND ${GENERATE_GLEAN_CMD}
)
set_target_properties(vpnglean_telemetry PROPERTIES FOLDER "Libs")

## Define depedencies
add_dependencies(vpnglean_ffi vpnglean_telemetry)
add_dependencies(vpnglean vpnglean_ffi)
set_target_properties(vpnglean PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
)
set_property(TARGET vpnglean APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated
)

## Fixup any missing link dependencies to pass to the vpnglean consumer
set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES -ldl)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenSSL REQUIRED)
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES OpenSSL::SSL
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    ## Windows seems to miss some system libraries when importing static OpenSSL
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES crypt32 Secur32 ntdll)
endif()
