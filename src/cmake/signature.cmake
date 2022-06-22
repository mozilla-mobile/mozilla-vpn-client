# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release)
    set(TARGET_DIR "release")
endif ()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(LIBNAME "signature.lib")
else()
    set(LIBNAME "libsignature.a")
endif()

get_filename_component(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR} ABSOLUTE)

add_custom_command(
   OUTPUT ${GENERATED_DIR}/${LIBNAME}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../signature/src/lib.rs
    COMMAND ${CARGO_CMD} --target-dir "${CMAKE_CURRENT_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../signature)

target_sources(mozillavpn PRIVATE
    ${GENERATED_DIR}/${LIBNAME}
)
target_link_libraries(mozillavpn PRIVATE ${GENERATED_DIR}/${LIBNAME})
target_link_libraries(mozillavpn PUBLIC ${CMAKE_DL_LIBS})

add_definitions(-DMVPN_SIGNATURE)
