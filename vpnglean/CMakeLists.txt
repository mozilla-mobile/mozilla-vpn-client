# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Find the absolute path cargo.
## Mostly necessary for iOS builds,
## because Xcode will attempt to use it's own installation of cargo,
## which we don't want.
find_program(CARGO_EXECUTABLE NAMES cargo REQUIRED)
find_program(RUSTC_EXECUTABLE NAMES rustc REQUIRED)

add_library(vpnglean STATIC IMPORTED GLOBAL)

if(ANDROID)
    add_dependencies(vpnglean ndk_openssl_merged)
    get_property(crypto_module GLOBAL PROPERTY OPENSSL_CRYPTO_MODULE)
    get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)
    get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)

    set(OPENSSL_LIBS ${openssl_libs})
    set(SSL_INCLUDE ${ssl_module}/include)
    set(SSL_LIB ${ssl_module}/libs/android.${ANDROID_ABI})
    set(CRYPTO_LIB ${crypto_module}/libs/android.${ANDROID_ABI})
endif()

set_target_properties(vpnglean PROPERTIES FOLDER "Libs")

# glean-core cannot be compiled to WASM
# See: https://blog.mozilla.org/data/2020/09/25/this-week-in-glean-glean-core-to-wasm-experiment/
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    # Figure out Rust's host architecture
    execute_process(OUTPUT_VARIABLE RUSTC_VERSION_RAW COMMAND rustc --version --verbose)
    if(RUSTC_VERSION_RAW MATCHES "host: ([^\n]+)")
        set(RUSTC_HOST_ARCH ${CMAKE_MATCH_1})
    else()
        message(FATAL_ERROR "Failed to find rustc host arch")
    endif()

    if(NOT CMAKE_CROSSCOMPILING)
        set(RUST_ARCH ${RUSTC_HOST_ARCH})
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
            set(RUST_ARCH "aarch64-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
            set(RUST_ARCH "armv7-linux-androideabi")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(RUST_ARCH "i686-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(RUST_ARCH "x86_64-linux-android")
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

    list(APPEND CARGO_ENV BUILD_ID=${BUILD_ID})
    list(APPEND CARGO_ENV APP_VERSION=${CMAKE_PROJECT_VERSION})
    list(APPEND CARGO_ENV CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR})

    if(ANDROID)
        list(APPEND CARGO_ENV
            OPENSSL_LIB_DIR=${OPENSSL_LIBS}/
            OPENSSL_INCLUDE_DIR=${SSL_INCLUDE}/
            AR=llvm-ar)
    elseif(APPLE AND XCODE)
        ## Don't trust Xcode to provide us with a usable linker.
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh "#!/bin/sh\n")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh "${RUSTC_EXECUTABLE} -C linker=/usr/bin/cc \$@\n")
        file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        )
        list(APPEND CARGO_ENV RUSTC=${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh)
    endif()

    set(HEADER_FILE vpnglean.h)
    set(LIBNAME ${CMAKE_STATIC_LIBRARY_PREFIX}vpnglean${CMAKE_STATIC_LIBRARY_SUFFIX})

    ## Build the vpnglean Rust library
    add_custom_target(vpnglean_ffi
        BYPRODUCTS
            ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
            ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
            ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
        COMMAND ${CMAKE_COMMAND} -E env ${CARGO_ENV}
                ${CARGO_EXECUTABLE} build $<IF:$<CONFIG:Debug>,,--release> --target ${RUST_ARCH}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    set_target_properties(vpnglean_ffi PROPERTIES FOLDER "Libs")

    add_dependencies(vpnglean_ffi vpnglean_telemetry)
    add_dependencies(vpnglean vpnglean_ffi)
else()
    message("Cannot build vpnglean Rust library build for WASM. Skipping.")
    # Since the metric recording code is sprinkled throughout the code,
    # it's easier to just generate the APIs and have them be noop.
    add_dependencies(mozillavpn vpnglean_telemetry)
endif()

# This options should be used if the glean_parser command needs to be run on some sort of pre-build script.
if(NOT SKIP_GLEAN_PARSER)
    set(GENERATE_GLEAN_CMD python3 ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py)

    # Prepare for generating Glean metrics and pings files
    # Note: the next steps are not required by run_glean_parser, but by CMake quirks.
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/generated)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/glean/generated)
    # 1. Because both the metrics.h and pings.h files have class declarations
    # of objects that extend QObject, these files need to be in the main MozillaVPN
    # target otherwise we run into linker issues e.g. undefined reference to `vtable for __DONOTUSE__GleanPings'.
    # See: https://github.com/bincrafters/community/issues/466#issuecomment-426329142
    #      and https://gitlab.cern.ch/-/snippets/2128
    # 2. Cmake is simply ignoring or being weird about adding these files as
    # sources to the main target and marking them as GENERATED TRUE.
    # See: https://gitlab.kitware.com/cmake/cmake/-/issues/18399
    #
    # The solution is to generate empty files at build setup time so that we can
    # add them as sources to other target and still not have Cmake freak out.
    # Actual generation happen when the `run_glean_parser` command is executed.
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/glean/generated/metrics.h)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/glean/generated/pings.h)
else()
    set(GENERATE_GLEAN_CMD echo "Skipping glean_parser step.")
endif()

## Generate metrics and pings files
add_custom_target(vpnglean_telemetry
    BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/glean/generated/metrics.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/apps/vpn/glean/generated/pings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/metrics.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/pings.rs
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py
        ${CMAKE_SOURCE_DIR}/glean/pings.yaml
        ${CMAKE_SOURCE_DIR}/glean/metrics.yaml
    COMMAND ${GENERATE_GLEAN_CMD}
)
set_target_properties(vpnglean_telemetry PROPERTIES FOLDER "Libs")

set_target_properties(vpnglean PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
)
set_property(TARGET vpnglean APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}
)

## Fixup any missing link dependencies to pass to the vpnglean consumer
set_property(TARGET vpnglean APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenSSL REQUIRED)
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES OpenSSL::SSL
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    ## Windows seems to miss some system libraries when importing static OpenSSL
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES crypt32 Secur32 bcrypt ncrypt ntdll)
endif()
