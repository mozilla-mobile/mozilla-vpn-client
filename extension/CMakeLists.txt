# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Custom target to build the web-extension bridge with Rust/Cargo
add_custom_target(cargo_mozillavpnnp ALL
    COMMENT "Building web extension crate"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bridge
    COMMAND ${CMAKE_COMMAND} -E env CARGO_HOME=${CMAKE_CURRENT_BINARY_DIR}/.cargo_home
            cargo build --release --target-dir ${CMAKE_CURRENT_BINARY_DIR}/bridge/target
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/bridge/target)

## Wrap the build artifacts as a CMake target.
add_executable(mozillavpnnp IMPORTED GLOBAL)
add_dependencies(mozillavpnnp cargo_mozillavpnnp)
set_property(TARGET mozillavpnnp PROPERTY
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/bridge/target/release/mozillavpnnp${CMAKE_EXECUTABLE_SUFFIX})

## Handle installation by platform.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    if(NOT DEFINED WEBEXT_INSTALL_LIBDIR)
        set(WEBEXT_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    endif()

    install(PROGRAMS $<TARGET_FILE:mozillavpnnp> DESTINATION ${WEBEXT_INSTALL_LIBDIR}/mozillavpn)

    configure_file(manifests/linux/mozillavpn.json.in manifests/linux/mozillavpn.json)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION ${WEBEXT_INSTALL_LIBDIR}/mozilla/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/opt/chrome/native-messaging-hosts)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/manifests/linux/mozillavpn.json
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/chromium/native-messaging-hosts)
elseif(WIN32)
    install(PROGRAMS $<TARGET_FILE:mozillavpnnp> DESTINATION .)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/manifests/windows/mozillavpn.json
        DESTINATION .)
elseif(APPLE)
    include(${CMAKE_SOURCE_DIR}/src/cmake/osxtools.cmake)
    osx_codesign_target(cargo_mozillavpnnp OPTIONS runtime FORCE FILES
        ${CMAKE_CURRENT_BINARY_DIR}/bridge/target/release/mozillavpnnp)
endif()
