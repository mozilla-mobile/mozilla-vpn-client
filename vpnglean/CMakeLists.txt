# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# glean-core cannot be compiled to WASM
# See: https://blog.mozilla.org/data/2020/09/25/this-week-in-glean-glean-core-to-wasm-experiment/
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    include(${CMAKE_SOURCE_DIR}/src/apps/vpn/cmake/rustlang.cmake)

    set(CARGO_ENV
        BUILD_ID=${BUILD_ID}
        APP_VERSION=${CMAKE_PROJECT_VERSION}
        CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # TODO: We should technically derive this from CMAKE_OSX_ARCHITECTURES
        set(RUST_ARCH aarch64-apple-darwin x86_64-apple-darwin)
    elseif(IOS)
        set(RUST_ARCH aarch64-apple-ios x86_64-apple-ios)
    elseif(ANDROID)
        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
            set(RUST_ARCH "aarch64-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
            set(RUST_ARCH "armv7-linux-androideabi")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(RUST_ARCH "i686-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(RUST_ARCH "x86_64-linux-android")
        endif()

        get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)
        get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)
        list(APPEND CARGO_ENV
            OPENSSL_LIB_DIR=${openssl_libs}
            OPENSSL_INCLUDE_DIR=${ssl_module}/include
            AR=llvm-ar)
    else()
        # Just use the host arch and hope for the best.
        set(RUST_ARCH ${RUSTC_HOST_ARCH})
    endif()

    ## Don't trust Xcode to provide us with a usable linker.
    if(APPLE AND XCODE)
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh "#!/bin/sh\n")
        file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh "${RUSTC_BUILD_TOOL} -C linker=/usr/bin/cc \$@\n")
        file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh FILE_PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_WRITE GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
        )
        list(APPEND CARGO_ENV RUSTC=${CMAKE_CURRENT_BINARY_DIR}/rustwrapper.sh)
    endif()

    set(HEADER_FILE vpnglean.h)

    ## Build the vpnglean Rust library
    add_rust_library(vpnglean
        ARCH ${RUST_ARCH}
        PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CRATE_NAME vpnglean
        CARGO_ENV ${CARGO_ENV}
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/metrics.rs
            ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/pings.rs
    )
else()
    message("Cannot build vpnglean Rust library build for WASM. Skipping.")
    # Since the metric recording code is sprinkled throughout the code,
    # it's easier to just generate the APIs and have them be noop.
    add_dependencies(mozillavpn vpnglean_telemetry)
endif()

# This options should be used if the glean_parser command needs to be run on some sort of pre-build script.
if(NOT SKIP_GLEAN_PARSER)
    set(GENERATE_GLEAN_CMD ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py)

    # Prepare for generating Glean metrics and pings files
    # Note: the next steps are not required by run_glean_parser, but by CMake quirks.
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/generated)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/apps/vpn/glean/generated)

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
    file(WRITE ${CMAKE_SOURCE_DIR}/src/apps/vpn/glean/generated/metrics.h)
    file(WRITE ${CMAKE_SOURCE_DIR}/src/apps/vpn/glean/generated/pings.h)
else()
    set(GENERATE_GLEAN_CMD echo "Skipping glean_parser step.")
endif()

## Generate metrics and pings files
add_custom_command(
    OUTPUT
        ${CMAKE_SOURCE_DIR}/src/apps/vpn/glean/generated/metrics.h
        ${CMAKE_SOURCE_DIR}/src/apps/vpn/glean/generated/pings.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/metrics.rs
        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/pings.rs
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py
        ${CMAKE_SOURCE_DIR}/glean/pings.yaml
        ${CMAKE_SOURCE_DIR}/glean/metrics.yaml
    COMMAND ${GENERATE_GLEAN_CMD}
)

set_property(TARGET vpnglean APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}
)

## Fixup any missing link dependencies to pass to the vpnglean consumer
if(ANDROID)
    add_dependencies(vpnglean ndk_openssl_merged)
elseif(LINUX)
    find_package(OpenSSL REQUIRED)
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES OpenSSL::SSL
    )
elseif(WIN32)
    ## Windows seems to miss some system libraries when importing static OpenSSL
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES crypt32 Secur32 bcrypt ncrypt ntdll)
endif()
