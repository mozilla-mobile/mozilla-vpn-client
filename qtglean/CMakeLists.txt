# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Let's check if the glean_parser module exists.
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "import glean_parser; print(glean_parser.__version__)"
    OUTPUT_VARIABLE GLEAN_PARSER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

## Add a static library for the Glean C++ code.
add_library(qtglean STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/boolean.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/datetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/quantity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/counter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/ping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/timingdistribution.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/uuid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/boolean.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/datetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/quantity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/counter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/ping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/timingdistribution.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/uuid.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.h
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.h
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.cpp
)

if(NOT MSVC AND NOT IOS)
  target_compile_options(qtglean PRIVATE -Wall -Werror -Wno-conversion)
endif()

target_include_directories(qtglean PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(qtglean PRIVATE Qt6::Core Qt6::Qml)

# glean-core cannot be compiled to WASM
# See: https://blog.mozilla.org/data/2020/09/25/this-week-in-glean-glean-core-to-wasm-experiment/
if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    include(${CMAKE_SOURCE_DIR}/scripts/cmake/rustlang.cmake)

    set(CARGO_ENV
        BUILD_ID=${BUILD_ID}
        APP_VERSION=${CMAKE_PROJECT_VERSION}
        CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
    )

    if(IOS)
        if (DEFINED CMAKE_OSX_ARCHITECTURES)
            if(${CMAKE_OSX_ARCHITECTURES} STREQUAL "arm64")
                set(RUST_ARCH aarch64-apple-ios)
            elseif(${CMAKE_OSX_ARCHITECTURES} STREQUAL "x86_64")
                set(RUST_ARCH x86_64-apple-ios)
            endif()
        endif()
        
        if (NOT DEFINED RUST_ARCH)
            set(RUST_ARCH aarch64-apple-ios x86_64-apple-ios)
        endif()

        message(FATAL_ERROR ${RUST_ARCH})
    elseif(ANDROID)
        file(READ ${CMAKE_CURRENT_SOURCE_DIR}/uniffi.toml.in UNIFFI_TOML_CONTENT)

        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
            set(RUST_ARCH "aarch64-linux-android")
            string(REPLACE "@ARCHITECTURE@" "arm64-v8a" UNIFFI_TOML_CONTENT ${UNIFFI_TOML_CONTENT})
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
            set(RUST_ARCH "armv7-linux-androideabi")
            string(REPLACE "@ARCHITECTURE@" "armeabi-v7a" UNIFFI_TOML_CONTENT ${UNIFFI_TOML_CONTENT})
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(RUST_ARCH "i686-linux-android")
            string(REPLACE "@ARCHITECTURE@" "x86" UNIFFI_TOML_CONTENT ${UNIFFI_TOML_CONTENT})
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(RUST_ARCH "x86_64-linux-android")
            string(REPLACE "@ARCHITECTURE@" "x86_64" UNIFFI_TOML_CONTENT ${UNIFFI_TOML_CONTENT})
        endif()

        # Prepare the uniffi config file for consumption by gradle.
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/uniffi.toml ${UNIFFI_TOML_CONTENT})

        get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)
        get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)
        list(APPEND CARGO_ENV
            OPENSSL_LIB_DIR=${openssl_libs}
            OPENSSL_INCLUDE_DIR=${ssl_module}/include
            AR=llvm-ar)
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


    ## Build the qtglean Rust library
    add_rust_library(qtglean_bindings
        ARCH ${RUST_ARCH}
        PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CRATE_NAME qtglean
        CARGO_ENV ${CARGO_ENV}
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.rs
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.rs
    )
    target_link_libraries(qtglean PRIVATE qtglean_bindings)

    set(HEADER_FILE qtglean.h)
else()
    message("Cannot build qtglean Rust library build for WASM. Skipping.")
    # Since the metric recording code is sprinkled throughout the code,
    # it's easier to just generate the APIs and have them be noop.
    add_custom_target(qtglean_telemetry
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.h
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.h
    )
    add_dependencies(${MAIN_TARGET} qtglean_telemetry)
endif()

set(GLEAN_PARSER_VERSION_MIN 5.0)
if(GLEAN_PARSER_VERSION VERSION_GREATER_EQUAL ${GLEAN_PARSER_VERSION_MIN})
    # If the glean parser exists, then we can build the telemetry bindings.
    set(GENERATE_GLEAN_CMD
        ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py
                --outdir ${CMAKE_CURRENT_BINARY_DIR}/glean/generated
    )
elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt)
    # On some build platforms, we may not have the ability to install the
    # glean_parser module, and we must rely on a pre-build script to generate
    # the telemetry bindings for us.
    set(GENERATE_GLEAN_CMD
        ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/glean/generated
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated
    )
elseif(GLEAN_PARSER_VERSION)
    message(FATAL_ERROR
        "Unsupported glean_parser version ${GLEAN_PARSER_VERSION} found.\n"
        "Please update to version ${GLEAN_PARSER_VERSION_MIN} or later."
    )
else()
    message(FATAL_ERROR
        "Glean generated files are missing, and the glean_parser was not found.\n"
        "Please run: python3 ./qtglean/glean_parser_ext/run_glean_parser.py"
    )
endif()

## Generate metrics and pings files
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.h
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.h
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.rs
        ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.rs
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/run_glean_parser.py
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/cpp_metrics_header.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/cpp_metrics_source.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/cpp_pings_header.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/cpp_pings_source.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/macros.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/rust_pings.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/glean_parser_ext/templates/rust.jinja2
        ${CMAKE_CURRENT_SOURCE_DIR}/metrics.yaml
        ${CMAKE_SOURCE_DIR}/glean/pings.yaml
        ${CMAKE_SOURCE_DIR}/glean/metrics.yaml
    COMMAND ${GENERATE_GLEAN_CMD}
)
set_source_files_properties(
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.h
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.h
    PROPERTIES GENERATED TRUE
)

## Fixup any missing link dependencies to pass to the qtglean consumer
if(ANDROID)
    add_dependencies(qtglean ndk_openssl_merged)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenSSL REQUIRED)
    set_property(TARGET qtglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES OpenSSL::SSL
    )
elseif(WIN32)
    ## Windows seems to miss some system libraries when importing static OpenSSL
    set_property(TARGET qtglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES crypt32 Secur32 bcrypt ncrypt ntdll)
endif()
