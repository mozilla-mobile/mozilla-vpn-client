# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(vpnglean STATIC IMPORTED GLOBAL)

set_target_properties(vpnglean PROPERTIES FOLDER "Libs")

set(HEADER_FILE vpnglean.h)
set(LIBNAME ${CMAKE_STATIC_LIBRARY_PREFIX}vpnglean${CMAKE_STATIC_LIBRARY_SUFFIX})

add_custom_target(vpnglean_ffi
    BYPRODUCTS
        ${CMAKE_CURRENT_BINARY_DIR}/debug/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/release/${LIBNAME}
        ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
    COMMAND ${CMAKE_COMMAND} -E env
                BUILD_ID=${BUILD_ID}
                APP_VERSION=${CMAKE_PROJECT_VERSION}
                CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}
            cargo build $<IF:$<CONFIG:Debug>,,--release>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(vpnglean_ffi PROPERTIES FOLDER "Libs")

add_dependencies(vpnglean vpnglean_ffi)
add_dependencies(mozillavpn vpnglean)
set_target_properties(vpnglean PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/release/${LIBNAME}
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/debug/${LIBNAME}
)

## TODO: It's not clear if we will also need this on other platforms, or
## if we just don't notice it due to statically linked Qt.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(OpenSSL REQUIRED)
    set_target_properties(vpnglean PROPERTIES
        IMPORTED_LINK_INTERFACE_LIBRARIES OpenSSL::SSL
endif()

target_include_directories(mozillavpn PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
