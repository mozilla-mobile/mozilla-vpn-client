# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(vpnglean STATIC IMPORTED GLOBAL)

set_target_properties(vpnglean PROPERTIES FOLDER "Libs")

set(RUST_ARCH "")
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(RUST_ARCH "x86_64-apple-darwin")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "ARM64")
        set(RUST_ARCH "aarch64-pc-windows-msvc")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
        set(RUST_ARCH "i686-pc-windows-msvc")
    elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
        set(RUST_ARCH "x86_64-pc-windows-msvc")
    endif()
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(RUST_ARCH "x86_64-unknown-linux-gnu")

    set(OPENSSL_USE_STATIC_LIBS TRUE)
    find_package(OpenSSL REQUIRED)
    get_filename_component(OPENSSL_LIB_DIR ${OPENSSL_SSL_LIBRARY} DIRECTORY)
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

    set(OPENSSL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/openSSL/static/lib/${OPENSSL_ARCH})
    set(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/openSSL/static/include)
endif()

if (RUST_ARCH STREQUAL "")
    message(FATAL_ERROR "Unable to determine Rust target for ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR}).")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release)
    set(TARGET_DIR "release")
endif ()

set(HEADER_FILE vpnglean.h)
set(LIBNAME ${CMAKE_STATIC_LIBRARY_PREFIX}vpnglean${CMAKE_STATIC_LIBRARY_SUFFIX})

add_custom_target(vpnglean_ffi
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
    COMMAND ${CMAKE_COMMAND} -E env
                BUILD_ID=${BUILD_ID}
                APP_VERSION=${CMAKE_PROJECT_VERSION}
                CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
            cargo build $<IF:$<CONFIG:Debug>,,--release> --target ${RUST_ARCH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(vpnglean_ffi PROPERTIES FOLDER "Libs")

add_dependencies(vpnglean vpnglean_ffi)
add_dependencies(mozillavpn vpnglean)
set_target_properties(vpnglean PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/release/${LIBNAME}
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/${RUST_ARCH}/debug/${LIBNAME}
)

## TODO: It's not clear if we will also need this on other platforms, or
## if we just don't notice it due to statically linked Qt.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(OpenSSL REQUIRED)
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES OpenSSL::SSL
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    ## Windows seems to miss some system libraries when importing static OpenSSL
    set_property(TARGET vpnglean APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES crypt32 Secur32)
endif()

target_include_directories(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
