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
set_target_properties(qtglean PROPERTIES FOLDER "Libs")
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

    # Build only the staticlib by default
    set(RUSTFLAGS "-Ctarget-feature=+crt-static")
    set(SHARED 0)

    if(IOS)
        set(RUST_ARCH "")
        if("arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            list(APPEND RUST_ARCH aarch64-apple-ios)
        endif()

        if("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            list(APPEND RUST_ARCH x86_64-apple-ios)
        endif()

        # If no architecure is defined, just do all of them.
        if (RUST_ARCH STREQUAL "")
            set(RUST_ARCH aarch64-apple-ios x86_64-apple-ios)
        endif()

        # On iOS we will build a dynamic lib,
        # so that it can be linked to both the main app and the network extension.
        set(RUSTFLAGS "-Ctarget-feature=-crt-static")
        set(SHARED 1)
        set(FW_NAME "QtGleanBindings")
    elseif(ANDROID)
        if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
            set(RUST_ARCH "aarch64-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7-a")
            set(RUST_ARCH "armv7-linux-androideabi")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
            set(RUST_ARCH "i686-linux-android")
        elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
            set(RUST_ARCH "x86_64-linux-android")
        else()
            message(FATAL_ERROR "Attempting to for unsupported architecture '${CMAKE_SYSTEM_PROCESSOR}'")
        endif()

        get_property(ssl_module GLOBAL PROPERTY OPENSSL_SSL_MODULE)
        get_property(openssl_libs GLOBAL PROPERTY OPENSSL_LIBS)
        list(APPEND CARGO_ENV
            OPENSSL_LIB_DIR=${openssl_libs}
            OPENSSL_INCLUDE_DIR=${ssl_module}/include
            AR=llvm-ar)

        # On Android we will build a dynamic lib,
        # so that it can be linked to both the main app and the daemon.
        set(RUSTFLAGS "-Ctarget-feature=-crt-static")
        set(SHARED 1)
    endif()

    ## Build the qtglean Rust library
    list(APPEND CARGO_ENV RUSTFLAGS=${RUSTFLAGS})
    add_rust_library(qtglean_bindings
        ARCH ${RUST_ARCH}
        PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CRATE_NAME qtglean
        CARGO_ENV ${CARGO_ENV}
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.rs
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.rs
        SHARED ${SHARED}
        FW_NAME ${FW_NAME}
    )

    target_link_libraries(qtglean PRIVATE qtglean_bindings)
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

list(APPEND PINGS_DEP_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/pings_deprecated.yaml)
list(APPEND PINGS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/pings.yaml)
list(APPEND METRICS_DEP_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/metrics_deprecated.yaml)
list(APPEND METRICS_LIST ${CMAKE_SOURCE_DIR}/src/shared/telemetry/metrics.yaml)

get_filename_component(APPS_DIR ${CMAKE_SOURCE_DIR}/src/apps ABSOLUTE)
file(GLOB APPS_NAMES LIST_DIRECTORIES true RELATIVE ${APPS_DIR} ${APPS_DIR}/*)
foreach(APP ${APPS_NAMES})
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
        list(APPEND PINGS_DEP_LIST ${APPS_DIR}/${APP}/telemetry/pings_deprecated.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/pings.yaml)
        list(APPEND PINGS_LIST ${APPS_DIR}/${APP}/telemetry/pings.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
        list(APPEND METRICS_DEP_LIST ${APPS_DIR}/${APP}/telemetry/metrics_deprecated.yaml)
    endif()
    if(EXISTS ${APPS_DIR}/${APP}/telemetry/metrics.yaml)
        list(APPEND METRICS_LIST ${APPS_DIR}/${APP}/telemetry/metrics.yaml)
    endif()
endforeach()

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
        ${PINGS_DEP_LIST}
        ${PINGS_LIST}
        ${METRICS_DEP_LIST}
        ${METRICS_LIST}
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
