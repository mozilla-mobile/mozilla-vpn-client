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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/basemetric.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/boolean.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/datetime.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/quantity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/counter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/customdistribution.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/ping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/timingdistribution.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/memorydistribution.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glean/uuid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/boolean.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/datetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/quantity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/counter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/customdistribution.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/ping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/timingdistribution.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/memorydistribution.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/uuid.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.h
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.h
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.cpp

)

# The reqwest library depends on SystemConfiguration for MacOS.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_library(FW_SYSTEMCONFIG SystemConfiguration)
    target_link_libraries(qtglean PRIVATE ${FW_SYSTEMCONFIG})
endif()

mz_target_handle_warnings(qtglean)

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
        # On iOS we will build a dynamic lib,
        # so that it can be linked to both the main app and the network extension.
        set(RUSTFLAGS "-Ctarget-feature=-crt-static")
        set(SHARED 1)
    elseif(ANDROID)
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
    elseif(QT_FEATURE_static AND QT_FEATURE_openssl AND QT_FEATURE_openssl_linked)
        # If Qt is statically linked to OpenSSL, so to should the glean libs.
        find_package(OpenSSL REQUIRED)
        get_filename_component(OPENSSL_LIB_DIR ${OPENSSL_SSL_LIBRARY} DIRECTORY)

        list(APPEND CARGO_ENV
            OPENSSL_STATIC=TRUE
            OPENSSL_LIB_DIR=${OPENSSL_LIB_DIR}
            OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR})
    endif()

    ## Build the qtglean Rust library
    list(APPEND CARGO_ENV RUSTFLAGS=${RUSTFLAGS})
    add_rust_library(qtglean_bindings
        PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        CARGO_ENV ${CARGO_ENV}
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/metrics.rs
            ${CMAKE_CURRENT_BINARY_DIR}/glean/generated/pings.rs
        SHARED ${SHARED}
    )

    ## For IOS targets - turn the rust library into a framework.
    if(IOS)
        set_target_properties(qtglean_bindings PROPERTIES
            FRAMEWORK TRUE
            OUTPUT_NAME QtGleanBindings
            MACOSX_FRAMEWORK_IDENTIFIER ${BUILD_IOS_APP_IDENTIFIER}.QtGleanBindings
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${BUILD_IOS_APP_IDENTIFIER}.QtGleanBindings
        )
    endif()

    target_link_libraries(qtglean PUBLIC qtglean_bindings)
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

list(APPEND PINGS_LIST ${CMAKE_SOURCE_DIR}/src/telemetry/pings.yaml)
list(APPEND METRICS_LIST
    ${CMAKE_SOURCE_DIR}/src/telemetry/metrics.yaml
    ${CMAKE_SOURCE_DIR}/src/telemetry/interaction_metrics.yaml
    ${CMAKE_SOURCE_DIR}/src/telemetry/impression_metrics.yaml
    ${CMAKE_SOURCE_DIR}/src/telemetry/outcome_metrics.yaml
)

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
