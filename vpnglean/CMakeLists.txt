# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(vpnglean STATIC IMPORTED GLOBAL)

set_target_properties(vpnglean PROPERTIES FOLDER "Libs")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
    cmake_path(GET OPENSSL_SSL_LIBRARY PARENT_PATH OPENSSL_LIB_DIR)
endif()

set(CARGO_BUILD_ENV
    BUILD_ID=${BUILD_ID}
    APP_VERSION=${CMAKE_PROJECT_VERSION}
    CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
    OPENSSL_STATIC=yes
    OPENSSL_LIB_DIR=${OPENSSL_LIB_DIR}
    OPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD ${CARGO_BUILD_ENV} cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD ${CARGO_BUILD_ENV} cargo build --release)
    set(TARGET_DIR "release")
endif ()

set (HEADER_FILE vpnglean.h)
set(LIBNAME ${CMAKE_STATIC_LIBRARY_PREFIX}vpnglean${CMAKE_STATIC_LIBRARY_SUFFIX})
get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR} ABSOLUTE)

add_custom_target(vpnglean_ffi
    BYPRODUCTS ${GENERATED_DIR}/${LIBNAME} ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
    COMMAND ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(vpnglean_ffi PROPERTIES FOLDER "Libs")

add_dependencies(vpnglean vpnglean_ffi)
add_dependencies(mozillavpn vpnglean)
set_target_properties(vpnglean PROPERTIES
    IMPORTED_LOCATION ${GENERATED_DIR}/${LIBNAME}
)

target_include_directories(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
