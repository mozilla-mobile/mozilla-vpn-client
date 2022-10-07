# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
    cmake_path(GET OPENSSL_SSL_LIBRARY PARENT_PATH OPENSSL_LIB_DIR)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Question: Why do I need to add these env vars to the command if they are already set by cmake?
    set(CARGO_CMD OPENSSL_STATIC=yes OPENSSL_LIB_DIR=${OPENSSL_LIB_DIR} OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR} cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD OPENSSL_STATIC=yes OPENSSL_LIB_DIR=${OPENSSL_LIB_DIR} OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR} cargo build --release)
    set(TARGET_DIR "release")
endif ()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(LIBNAME "vpnglean.lib")
else()
    set(LIBNAME "libvpnglean.a")
endif()

get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR} ABSOLUTE)

# TODO: The following code assumes glean_parser and jinja2 will be installed globally,
# we probably want to create a virtualenv somehow and check for Python like we check for OpenSSL.

# Generate the Glean Rust artifacts
add_custom_command(
    OUTPUT ${GENERATED_DIR}/${LIBNAME}
    BYPRODUCTS vpnglean/src/generated/pings.rs vpnglean/src/generated/metrics.rs

    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean/src/lib.rs
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean/metrics.yaml ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean/pings.yaml
    COMMAND rm -rf src/generated
    COMMAND mkdir -p src/generated
    COMMAND python3 glean_parser_ext/run_glean_parser.py rust pings.yaml >> src/generated/pings.rs
    COMMAND python3 glean_parser_ext/run_glean_parser.py rust metrics.yaml >> src/generated/metrics.rs
    COMMAND ${CARGO_CMD} --target-dir "${CMAKE_CURRENT_BINARY_DIR}"

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean
)

# Generate the Glean CPP artifacts
add_custom_target(glean_cpp_artifacts
    BYPRODUCTS src/glean/extern.h src/glean/generated/metrics.cpp src/glean/generated/pings.cpp

    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean/metrics.yaml ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean/pings.yaml
    COMMAND rm -rf ../src/glean/generated
    COMMAND mkdir -p ../src/glean/generated
    COMMAND python3 glean_parser_ext/run_glean_parser.py cpp pings.yaml >> ../src/glean/generated/pings.h
    COMMAND python3 glean_parser_ext/run_glean_parser.py cpp metrics.yaml >> ../src/glean/generated/metrics.h
    COMMAND cbindgen --config cbindgen.toml --crate vpnglean --output ../src/glean/extern.h

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../vpnglean
)

set_target_properties(glean_cpp_artifacts PROPERTIES FOLDER "Libs")
add_dependencies(mozillavpn glean_cpp_artifacts)

set_source_files_properties(
    src/glean/extern.h
    src/glean/generated/metrics.h
    src/glean/generated/pings.h
    PROPERTIES GENERATED TRUE
)

target_sources(mozillavpn PRIVATE
    ${GENERATED_DIR}/${LIBNAME}

)
target_link_libraries(mozillavpn PRIVATE ${GENERATED_DIR}/${LIBNAME})
target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

add_definitions(-DMVPN_VPNGLEAN)
